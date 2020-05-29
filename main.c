#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define CLIENT_COUNT 10     //Clients Limit
#define SHAVING_TIME 2  //Shaving end time

int armchair_count = 0;     //Variables
int client_count = 0;
int chair_count = 0;
int free_chair_count = 0;
int serve_client = 0;
int sit_chair = 0;

int* armchair;

sem_t barbers_sem;  //Semaphores to be used
sem_t clients_sem;
sem_t mutex_sem;

void Barber(void* num)
{
    int barber_id = *(int*)num +1; //Barber_id is assigned to the Barber function as a parameter
    int nextClient, musteri_id;

    printf("%d. Barber entered the salon.\n",barber_id);

    while(1)    //First, it is checked whether there are clients in the salon
    {
        if (!musteri_id)
        {
            printf("%d. Barber slept.\n\n", barber_id);     //Barber sleeps if there is no client
        }
        sem_wait(&barbers_sem);     //Access to the seat where there is no client is blocked.
        sem_wait(&mutex_sem);

        serve_client = (++serve_client) % chair_count;  //The client to shave becomes one of the waiting clients.
        nextClient = serve_client;
        musteri_id = armchair[nextClient];
        armchair[nextClient] = pthread_self();

        sem_post(&mutex_sem);   //Access to the chair is opened and the barber shaves the client.
        sem_post(&clients_sem);

        printf("%d. Barber %d. started shaving the client.\n\n", barber_id, musteri_id);   //After the specified shaving time, the shaving ends and the thread ends.
        sleep(SHAVING_TIME);
        printf("%d. Barber %d. finished shaving the client.\n\n",barber_id, musteri_id);
    }
    pthread_exit(0);
}

void Client(void* num)  //Takes client number as parameter in Client function
{
    int n = *(int*)num +1;
    int seatedChair, barber_identity;

    sem_wait(&mutex_sem);   //Access to the seat is locked

    printf("%d. Client came to the salon.\n", n);      //Incoming client is printed on the screen

    if (free_chair_count > 0)
    {
        free_chair_count--;     //The number of chairs is reduced by one

        printf("%d. Client waiting.\n\n", n);

        sit_chair = (++sit_chair) % chair_count;
        seatedChair = sit_chair;
        armchair[seatedChair] = n;     //Client is allowed to sit on one of the empty chairs

        sem_post(&mutex_sem);
        sem_post(&barbers_sem);
                                    //Access to the chair is removed and the sleeping barber is awakened
        sem_wait(&clients_sem);
        sem_wait(&mutex_sem);

        barber_identity = armchair[seatedChair];    //Barber takes care of the client
        free_chair_count++;

        sem_post(&mutex_sem);
    }
    else
    {
        sem_post(&mutex_sem);
        printf("%d. Client couldn't find seat to wait. Leving the salon.\n\n", n);  //If there is no empty chair, the client leaves the salon
    }
    pthread_exit(0);
}

void Wait()
{
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 +1) + 50000);
}

int main (int argc, char** args)
{
    printf("Enter the number of clients : ");   //To request the number of clients, armchairs and chairs to be used from users
    scanf("%d", &client_count);

    printf("Enter the number of chair: ");
    scanf("%d", &chair_count);

    printf("Enter the number of armchair: ");
    scanf("%d", &armchair_count);

    free_chair_count = chair_count;
    armchair = (int*) malloc(sizeof(int) * chair_count);    //To create a set of armchairs that we set the limit

    if (client_count > CLIENT_COUNT)
    {
        printf("\nClient limit: %d\n\n", CLIENT_COUNT);
        return EXIT_FAILURE;
    }

    pthread_t barber[armchair_count], client[client_count];     //Creating barber and client thread variables

    sem_init(&barbers_sem, 0, 0);   //Starting semaphores with the Semi method
    sem_init(&clients_sem, 0, 0);
    sem_init(&mutex_sem,0, 1);

    printf("\nBarber opening the salon.\n\n");

    for(int i = 0; i<armchair_count; i++)
    {
        pthread_create(&barber[i], NULL, (void*)Barber, (void*)&i);     //Creating barber and client threads with pthread_create method
        sleep(1);
    }

    for(int i = 0; i < client_count; i++)
    {
        pthread_create(&client[i], NULL, (void*)Client, (void*)&i);
        Wait();
    }

    for(int i = 0; i < client_count; i++)
    {
        pthread_join(client[i], NULL);
    }

    printf("\nThe trades of all clients are over. The barbers closed the salon!\n\n");  //Barber closes the salon

    return EXIT_SUCCESS;
}
