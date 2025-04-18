// Heat2D.cpp
// MPI + OpenACC + Boost.Program_options

#include <mpi.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <iomanip>
#include <boost/program_options.hpp>


namespace po = boost::program_options;

#define IND(i, j) ((i) * (local_nx + 2) + (j))

int get_block_size(int n, int rank, int nprocs) {
    int s = n / nprocs;
    if (n % nprocs > rank)
        s++;
    return s;
}

int get_sum_of_prev_blocks(int n, int rank, int nprocs) {
    int rem = n % nprocs;
    return n / nprocs * rank + ((rank >= rem) ? rem : rank);
}

int main(int argc, char* argv[]) {
    int commsize, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Command line options
    int nx, ny, max_iters;
    double eps;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("nx", po::value<int>(&nx)->default_value(128), "number of columns")
        ("ny", po::value<int>(&ny)->default_value(128), "number of rows")
        ("eps", po::value<double>(&eps)->default_value(1e-6), "error threshold")
        ("max-iters", po::value<int>(&max_iters)->default_value(1000000), "maximum iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        if (rank == 0) std::cout << desc << "\n";
        MPI_Finalize();
        return 0;
    }

    // Create 2D process grid
    int dims[2] = {0, 0}, periodic[2] = {0, 0};
    MPI_Dims_create(commsize, 2, dims);
    int px = dims[0], py = dims[1];
    if (px < 2 || py < 2) {
        if (rank == 0) std::cerr << "Need at least 2x2 process grid\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm cartcomm;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodic, 0, &cartcomm);
    int coords[2];
    MPI_Cart_coords(cartcomm, rank, 2, coords);
    int rankx = coords[0], ranky = coords[1];

    int local_nx = get_block_size(nx, rankx, px);
    int local_ny = get_block_size(ny, ranky, py);
    int global_sx = get_sum_of_prev_blocks(nx, rankx, px);
    int global_sy = get_sum_of_prev_blocks(ny, ranky, py);

    std::vector<double> grid((local_ny + 2) * (local_nx + 2), 0.0);
    std::vector<double> newgrid((local_ny + 2) * (local_nx + 2), 0.0);

    // Corner values for interpolation
    double c00 = 10.0, c01 = 20.0, c11 = 30.0, c10 = 20.0;

    double dx = 1.0 / (nx - 1);
    double dy = 1.0 / (ny - 1);

    // Interpolate borders
    for (int j = 1; j <= local_nx; ++j) {
        int gj = global_sx + j - 1;
        double x = dx * gj;
        if (ranky == 0) {
            double val = c00 * (1 - x) + c01 * x;
            grid[IND(0, j)] = newgrid[IND(0, j)] = val;
        }
        if (ranky == py - 1) {
            double val = c10 * (1 - x) + c11 * x;
            grid[IND(local_ny + 1, j)] = newgrid[IND(local_ny + 1, j)] = val;
        }
    }
    for (int i = 1; i <= local_ny; ++i) {
        int gi = global_sy + i - 1;
        double y = dy * gi;
        if (rankx == 0) {
            double val = c00 * (1 - y) + c10 * y;
            grid[IND(i, 0)] = newgrid[IND(i, 0)] = val;
        }
        if (rankx == px - 1) {
            double val = c01 * (1 - y) + c11 * y;
            grid[IND(i, local_nx + 1)] = newgrid[IND(i, local_nx + 1)] = val;
        }
    }

    MPI_Datatype row_type, col_type;
    MPI_Type_contiguous(local_nx, MPI_DOUBLE, &row_type);
    MPI_Type_vector(local_ny, 1, local_nx + 2, MPI_DOUBLE, &col_type);
    MPI_Type_commit(&row_type);
    MPI_Type_commit(&col_type);

    int top, bottom, left, right;
    MPI_Cart_shift(cartcomm, 1, 1, &top, &bottom);
    MPI_Cart_shift(cartcomm, 0, 1, &left, &right);

    double diff = 0;
    int iter = 0;
    double t0 = MPI_Wtime();
    
    #pragma acc data copy(grid[0:(local_ny+2)*(local_nx+2)], newgrid[0:(local_ny+2)*(local_nx+2)])
    {
        while (iter++ < max_iters) {
            diff = 0.0;
            #pragma acc parallel loop collapse(2) present(grid, newgrid) reduction(max:diff)
            for (int i = 1; i <= local_ny; ++i) {
                for (int j = 1; j <= local_nx; ++j) {
                    newgrid[IND(i, j)] = 0.25 * (grid[IND(i-1, j)] + grid[IND(i+1, j)] +
                                                 grid[IND(i, j-1)] + grid[IND(i, j+1)]);
                    diff = fmax(diff, fabs(grid[IND(i, j)] - newgrid[IND(i, j)]));
                }
            }
    
            std::swap(grid, newgrid);
            #pragma acc update device(grid[0:(local_ny+2)*(local_nx+2)], newgrid[0:(local_ny+2)*(local_nx+2)])
            double global_diff;
            MPI_Allreduce(&diff, &global_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
            if (global_diff < eps) break;
    
            // Halo exchange
            MPI_Request reqs[8];
            MPI_Irecv(&grid[IND(0,1)], 1, row_type, top, 0, cartcomm, &reqs[0]);
            MPI_Irecv(&grid[IND(local_ny+1,1)], 1, row_type, bottom, 0, cartcomm, &reqs[1]);
            MPI_Irecv(&grid[IND(1,0)], 1, col_type, left, 0, cartcomm, &reqs[2]);
            MPI_Irecv(&grid[IND(1,local_nx+1)], 1, col_type, right, 0, cartcomm, &reqs[3]);
            MPI_Isend(&grid[IND(1,1)], 1, row_type, top, 0, cartcomm, &reqs[4]);
            MPI_Isend(&grid[IND(local_ny,1)], 1, row_type, bottom, 0, cartcomm, &reqs[5]);
            MPI_Isend(&grid[IND(1,1)], 1, col_type, left, 0, cartcomm, &reqs[6]);
            MPI_Isend(&grid[IND(1,local_nx)], 1, col_type, right, 0, cartcomm, &reqs[7]);
            MPI_Waitall(8, reqs, MPI_STATUS_IGNORE);
        }
    }
    
    double t1 = MPI_Wtime();
    
    if (rank == 0)
        std::cout << "# Iters: " << iter << ", Error: " << diff << ", Time: " << (t1 - t0) << "s\n";
    // Print grid if small
    if ((nx == 10 || nx == 13) && (ny == 10 || ny == 13)) {
        for (int p = 0; p < commsize; ++p) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (rank == p) {
                std::cout << "Rank " << rank << " block (" << local_ny << " x " << local_nx << ")\n";
                for (int i = 1; i <= local_ny; ++i) {
                    for (int j = 1; j <= local_nx; ++j)
                        std::cout << std::fixed << std::setprecision(2) << grid[IND(i, j)] << " ";
                    std::cout << "\n";
                }
                std::cout << std::flush;
            }
        }
    }

    MPI_Type_free(&row_type);
    MPI_Type_free(&col_type);
    MPI_Finalize();
    return 0;
}
