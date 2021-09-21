#include <mpi.h>
#include <stdio.h>
#include <string>
#include "ctime"

#define N 10
#define MAX_LEN 10

void writeMoneyAmount(int rank, int i, MPI_Status status) {
    char good[MAX_LEN];
    MPI_Recv(good, MAX_LEN, MPI_CHAR, 0, i, MPI_COMM_WORLD, &status);
    // распаковка (смысла в ней здесь нет)
    char unpacked[MAX_LEN];
    int pos = 0;
    MPI_Unpack(good, MAX_LEN, &pos, unpacked, MAX_LEN, MPI_CHAR, MPI_COMM_WORLD);
    int res = rand() % 1000;
    // Отправка суммы денег распорядителю
    printf("The client %d wants to pay %d$ for the %c\n", rank, res, unpacked[0]);

    int packed[1];
    pos = 0;
    MPI_Pack(new int[1]{res}, 1, MPI_INT, packed, 5, &pos, MPI_COMM_WORLD);
    MPI_Send(packed, 1, MPI_INT, 0, i, MPI_COMM_WORLD);
}

int main(int argc, char **argv) {
    char goods[N] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
    double t, t2;
    int rank, size;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

    if (rank == 0) {
        double totalTime = 0;
        for (int i = 0; i < N; ++i) {
            char tmp[MAX_LEN];
            int pos = 0;
            for (int j = 0; j < MAX_LEN; ++j) {
                tmp[j] = goods[i];
            }
            // упаковка (смысла в ней здесь нет)
            char packed[MAX_LEN];
            MPI_Pack(&tmp, MAX_LEN, MPI_CHAR, &packed, MAX_LEN, &pos, MPI_COMM_WORLD);
            t = MPI_Wtime();
            // Рассказ участникам аукциона о каждом лоте
            for (int j = 1; j < size; ++j) {
                //посылка упакованных данных
                MPI_Send(packed, MAX_LEN, MPI_CHAR, j, i, MPI_COMM_WORLD);
            }
            // Сбор карточек с суммой и выбор победителя
            int max_bid = 0;
            int winner = 0;
            int bid[MAX_LEN];
            for (int j = 1; j < size; ++j) {
                MPI_Recv(bid, MAX_LEN, MPI_CHAR, j, i, MPI_COMM_WORLD, &status);
                pos = 0;
                int unpacked[1];
                MPI_Unpack(bid, 5, &pos, &unpacked, 1, MPI_INT, MPI_COMM_WORLD);
                if (unpacked[0] > max_bid) {
                    max_bid = unpacked[0];
                    winner = j;
                }
            }
            printf("The client %d is winner! %c sold for %d$\n", winner, goods[i], max_bid);
            t2 = MPI_Wtime();
            totalTime += t2 - t;
        }
        printf("\nTotal time=%f ms\n",  totalTime * 1000 / 10); // вывод времени в мс для одного "проданного лота"
    } else {
        for (int i = 0; i < N; ++i) {
            writeMoneyAmount(rank, i, status);
        }
    }

    MPI_Finalize();

    return 0;
}