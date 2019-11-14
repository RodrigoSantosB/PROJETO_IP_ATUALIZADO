#include "bola.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define MSG_MAX_SIZE 350
#define BUFFER_SIZE (MSG_MAX_SIZE + 100)
#define LOGIN_MAX_SIZE 13
#define HIST_MAX_SIZE 200

#define MAX_LOG_SIZE 17

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;

bool inicializar();

enum conn_ret_t tryConnect() {
  char server_ip[30];
  printf("Please enter the server IP: ");
  scanf(" %s", server_ip);
  getchar();
  return connectToServer(server_ip);
}

void printHello() {
  puts("Hello! Wellcome to simple chat example.");
  puts("We need some infos to start up!");
}

void assertConnection(char login[]) {
  printHello();
  enum conn_ret_t ans = tryConnect();

  while (ans != SERVER_UP)  {
    if (ans == SERVER_DOWN) {
      puts("Server is down!");
    } else if (ans == SERVER_FULL) {
      puts("Server is full!");
    } else if (ans == SERVER_CLOSED) {
      puts("Server is closed for new connections!");
    } else {
      puts("Server didn't respond to connection!");
    }
    printf("Want to try again? [Y/n] ");
    int res;
    while (res = tolower(getchar()), res != 'n' && res != 'y' && res != '\n'){
      puts("anh???");
    }
    if (res == 'n') {
      exit(EXIT_SUCCESS);
    }
    ans = tryConnect();
  }
  int len = (int)strlen(login);
  sendMsgToServer(login, len + 1);
}

void runChat() {
  char str_buffer[BUFFER_SIZE], type_buffer[MSG_MAX_SIZE] = {0};
  char msg_history[HIST_MAX_SIZE][MSG_MAX_SIZE] = {{0}};
  int type_pointer = 0;
  
  while (1) {
    // LER UMA TECLA DIGITADA
    char ch = getch();
    if (ch == '\n') {
      type_buffer[type_pointer++] = '\0';
      int ret = sendMsgToServer((void *)type_buffer, type_pointer);
      if (ret == SERVER_DISCONNECTED) {
        return;
      }
      type_pointer = 0;
      type_buffer[type_pointer] = '\0';
    } else if (ch == 127 || ch == 8) {
      if (type_pointer > 0) {
        --type_pointer;
        type_buffer[type_pointer] = '\0';
      }
    } else if (ch != NO_KEY_PRESSED && type_pointer + 1 < MSG_MAX_SIZE) {
      type_buffer[type_pointer++] = ch;
      type_buffer[type_pointer] = '\0';
      
    }

    // LER UMA MENSAGEM DO SERVIDOR
    int ret = recvMsgFromServer(str_buffer, DONT_WAIT);
    if (ret == SERVER_DISCONNECTED) {
      return;
    } else if (ret != NO_MESSAGE) {
      int i;
      for (i = 0; i < HIST_MAX_SIZE - 1; ++i) {
        strcpy(msg_history[i], msg_history[i + 1]);
      }
      strcpy(msg_history[HIST_MAX_SIZE - 1], str_buffer);
    }

    // PRINTAR NOVO ESTADO DO CHAT
    system("clear");
    int i;
    for (i = 0; i < HIST_MAX_SIZE; ++i) {
      printf("%s\n", msg_history[i]);
    }
    printf("\nYour message: %s\n", type_buffer);
  }
}

int main() {
  char login[30], ip[20];
  puts("Digite seu login!");
  scanf(" %s", login);
  assertConnection(login);
  bool sair = false;

  if (!inicializar()) {
    return -1;
  }
  while (!sair) {
    if (!al_is_event_queue_empty(fila_eventos))
    {
      ALLEGRO_EVENT evento;
      al_wait_for_event(fila_eventos, &evento);

      if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
        sair = true;
      }
    }
    center_t center;
    int result = recvMsgFromServer(&center, DONT_WAIT);
    if (result == SERVER_DISCONNECTED) {
      puts("server disconnected!!!");
      exit(EXIT_SUCCESS);
    } else if (result != NO_MESSAGE) {
      al_draw_filled_circle(center.x, center.y, RADIUS, al_map_rgb(255, 0, 0));
      al_flip_display();
      al_clear_to_color(al_map_rgb(0, 0, 0)); 
    }
  }
  return 0;
}


bool inicializar()
{
  if (!al_init()) {
    fprintf(stderr, "Falha ao inicializar Allegro.\n");
    return false;
  }

  if (!al_init_primitives_addon()) {
    fprintf(stderr, "Falha ao inicializar add-on allegro_primitives.\n");
    return false;
  }

  janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
  if (!janela) {
    fprintf(stderr, "Falha ao criar janela.\n");
    return false;
  }

  al_set_window_title(janela, "Animando!!!");

  fila_eventos = al_create_event_queue();
  if (!fila_eventos) {
    fprintf(stderr, "Falha ao criar fila de eventos.\n");
    al_destroy_display(janela);
    return false;
  }

  al_register_event_source(fila_eventos, al_get_display_event_source(janela));
  return true;
}
