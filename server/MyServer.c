#include "bola.h"
#include "server.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define MAX_CHAT_CLIENTS 3
#define UPDATE_RATE 0.005

double calc_time(clock_t start, clock_t end) {
  return ((double) (end - start)) / CLOCKS_PER_SEC;
}

int main() {
  char client_names[MAX_CHAT_CLIENTS][LOGIN_MAX_SIZE];
  char str_buffer[BUFFER_SIZE], aux_buffer[BUFFER_SIZE];
  serverInit(MAX_CHAT_CLIENTS);
  puts("Server is running!!");
  clock_t last_ball_update = clock();
  double x = RADIUS;
  double y = RADIUS;
  int dir_x = 1, dir_y = 1;

  while (1) {
    int id = acceptConnection();
    if (id != NO_CONNECTION) {
      recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
      printf("%s connected id = %d\n", client_names[id], id);
      center_t center = {x, y};
      sendMsgToClient(&center, sizeof(center), id);
    }

    struct msg_ret_t result = recvMsg(NULL);
    if (result.status == DISCONNECT_MSG) {
      printf("The user %s disconnected!\n", client_names[result.client_id]);
    }

    if (calc_time(last_ball_update, clock()) >= UPDATE_RATE) {
      x += 1.0 * dir_x;
      y += 1.0 * dir_y;        

      if (x >= LARGURA_TELA - RADIUS) {
        dir_x = -1;
        x = LARGURA_TELA - RADIUS;
      }
      else if (x <= RADIUS)  {
        dir_x = 1;
        x = RADIUS;
      }

      if (y >= ALTURA_TELA - RADIUS) {
        dir_y = -1;
        y = ALTURA_TELA - RADIUS;
      } 
      else if (y <= RADIUS)  {
        dir_y = 1;
        y = RADIUS;
      }
      center_t center = {x, y};
      broadcast(&center, sizeof(center));
      last_ball_update = clock();
    }
  }
}
