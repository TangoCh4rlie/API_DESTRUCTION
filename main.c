#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 8

struct ThreadData {
  char *json_data;
  int nb_times;
};

void *perform_request(void *arg) {
  struct ThreadData *threadData = (struct ThreadData *)arg;
  char *data = threadData->json_data;
  int nb_times = threadData->nb_times;

  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://serrata-stats.super-sympa.fr/api/events");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    for (int i = 0; i < nb_times; i++) {
      printf("%d fois", i);
      res = curl_easy_perform(curl);

      if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        exit(-1);
      }
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  free(arg);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int nb_threads = atoi(argv[2]);
  pthread_t threads[nb_threads];

  curl_global_init(CURL_GLOBAL_DEFAULT);

  for (int i = 0; i < nb_threads; i++) {
    struct ThreadData *threadData =
        (struct ThreadData *)malloc(sizeof(struct ThreadData));
    if (!threadData) {
      fprintf(stderr,
              "Erreur lors de l'allocation de la mémoire pour threadData\n");
      exit(-1);
    }
    threadData->json_data =
        "{\"event\":{\"type\":\"testpourvoirsielixirestsibienqueca\",\"user_"
        "id\":1,\"data\":{\"key1\":\"value1\"}}}";
    threadData->nb_times = atoi(argv[1]);

    int rc =
        pthread_create(&threads[i], NULL, perform_request, (void *)threadData);
    if (rc) {
      fprintf(stderr,
              "Erreur lors de la création du thread %d; code de retour %d\n", i,
              rc);
      exit(-1);
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  curl_global_cleanup();

  return 0;
}
