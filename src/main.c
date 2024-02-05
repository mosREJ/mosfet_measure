#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <argp.h>
#include <errno.h>
#include <string.h>
#include "tcp.h"
#include <math.h>
#include "serial.h"
#include "cmds.h"
#include "ring_buffer.h"

#ifndef EXIT_SUCCESS 
#define EXIT_SUCCESS 0
#endif /* EXIT_SUCCESS */
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif /* EXIT_FAILURE */

static const char prog_desc[] = "Use remote controlled power supply and multimeter to measure MOSFET characteristics";

static const char args_doc[] = "ARG1 ARG2";

static const struct argp_option options[] = {
    { "config", 'c', "FILEPATH", OPTION_ARG_OPTIONAL, "Use config file FILEPATH" },
    { "powersupply-serial", 'p', "DEVICEPATH", OPTION_ARG_OPTIONAL, "Use power supply device connected over USB/RS232" },
    { "pwbaudrate", 'b', "BAUDRATE", OPTION_ARG_OPTIONAL, "Use power supply device connected over USB/RS232" },
    { "powersupply", 'P', "IPADDR", OPTION_ARG_OPTIONAL, "Use remote controlled power supply over TCP" }, { "pwport", 'o', "PORT", OPTION_ARG_OPTIONAL, "Power supply TCP port" },
    { "multimeter-serial", 'm', "DEVICEPATH", OPTION_ARG_OPTIONAL, "Use multimeter connected over USB/RS232" },
    { "mmbaudrate", 'B', "BAUDRATE", OPTION_ARG_OPTIONAL, "Baudrate for a multimeter connected over USB/RS232" },
    { "multimeter", 'M', "IPADDR", OPTION_ARG_OPTIONAL, "User remote controlled multimeter over TCP" }, 
    { "mmport", 'O', "PORT", OPTION_ARG_OPTIONAL, "Multimeter TCP port" },
    { "3dprinter-serial", 'd', "DEVICEPATH", OPTION_ARG_OPTIONAL, "Use 3dprinter device connected over USB/RS232"},
    { "3dpbaudrate", 'w', "BAUDRATE", OPTION_ARG_OPTIONAL, "Use 3dprinter device connected over USB/RS232"},
    { 0 }
};

struct prog_args
{
    union {
        struct {
            char* devpath;
            int baudrate;
        };
        struct {
            char* ipaddr;
            int port; 
        };
    } powersupply;
    int ps_uses_inet;

    union {
        struct {
            char* devpath;
            int baudrate;
        };
        struct {
            char* ipaddr;
            int port; 
        };
    } multimeter;
    int mm_uses_inet;
  
    union {
        struct {
            char* devpath;
            int baudrate;
        };
        struct {
            char* ipaddr;
            int port; 
        };
    } printer;
    int printer_uses_inet;

    char* config_path;    
};

static error_t parse_opt(int key, char* arg, struct argp_state* state);

static struct argp argp = { options, parse_opt, args_doc, prog_desc };

static void sigint_handler(int sig);

static int ps_fd = -1, mm_fd = -1, printer_fd = -1;

static int open_devices(struct prog_args* args);

struct recvt_args
{
    int fd;   
    struct ring_buffer_t* rb;            
};

static void* receiver_thread(void* arg);

static pthread_t mm_recvt, ps_recvt; // at his moment we dont rx anything from printer

static struct ring_buffer_t mm_rb, ps_rb;

#define RB_SIZE 1024 

static void print_recvd_chars()
{
    char c;
    while(rb_pop(&mm_rb, &c) != -1) 
        putchar(c);
    while(rb_pop(&ps_rb, &c) != -1) 
        putchar(c);
}

int main(int argc, char** argv) 
{
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    struct prog_args args;
    
    args.config_path = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    if(open_devices(&args) == EXIT_FAILURE)
        return EXIT_FAILURE;  
    
    rb_init(&mm_rb, RB_SIZE);
    rb_init(&ps_rb, RB_SIZE); 

    struct recvt_args mm_recvt_args = {
        .fd = mm_fd,
        .rb = &mm_rb
    }; 

    struct recvt_args ps_recvt_args = {
        .fd = ps_fd,
        .rb = &ps_rb
    };

    if(pthread_create(&mm_recvt, NULL, receiver_thread, (void*)&mm_recvt_args) == -1)
        goto pthread_fail;
    if(pthread_create(&ps_recvt, NULL, receiver_thread, (void*)&ps_recvt_args) == -1)
        goto pthread_fail;
   
    /* 
     * ofc all sequence (FSM) need to be implemented e.g. reset value of mm -> set current limit -> Turn on -> measure -> reset mm -> turn off */



/* unit: [mm] */
    #define Z_STOP          2.5
    #define Z_START         4.5
    #define X_STEP          0.1
    #define Y_STEP          0.1
    #define WAFER_LEN       15.2         
    #define WAFER_WIDTH     9.7
    #define Y_START         100.0
    #define Y_STOP          70.0
    #define X_START         100.0
    #define X_STOP          70.0
    /*          */
    
    /* unit: [s] */
    #define DELAY_BTW_MES   1
    #define DELAY_INIT      4
    /*          */
    
    #define measure_per_pos   10
    const int step_cnt_x = fabs((X_START - X_STOP) / X_STEP);
    const int step_cnt_y = fabs((Y_START - Y_STOP) / Y_STEP);
    
    PRINT_MOVE_G1_Y(Y_START);
    PRINT_MOVE_G1_X(X_START);
    PRINT_MOVE_G1_Z(Z_START);
    MEAS_SET_OPT_FILTER();
    PS_ENABLE_OUT();
    sleep(DELAY_INIT);

   for (int curr_x = 0; curr_x < step_cnt_x; curr_x++)
   {
       ;
       for (int curr_y = 0; curr_y < step_cnt_y; curr_y++)
       {

           for (int i = 0; i < measure_per_pos; i++)
           {

           }


       }



   }


    PS_DISABLE_OUT();
    PRINT_MOVE_G1_Y(Y_START);
    PRINT_MOVE_G1_X(X_START);

    




    
    pthread_kill(mm_recvt, SIGTERM);
    pthread_kill(ps_recvt, SIGTERM);
pthread_fail:
    close(ps_fd);
    close(mm_fd);
    return EXIT_SUCCESS;
}

static int open_devices(struct prog_args* args)
{ 
    assert(args);

    if(args->mm_uses_inet) {
        mm_fd = tcp_connect(args->multimeter.ipaddr, args->multimeter.port);   
    } else {
        speed_t baudrate = args->multimeter.baudrate == 9600 ? B9600 : B115200;
        mm_fd = serial_open(args->multimeter.devpath, baudrate); 
    }
    
    if(mm_fd == -1) {
        fprintf(stderr, "Error while connecting to multimeter: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if(args->ps_uses_inet) {
        ps_fd = tcp_connect(args->powersupply.ipaddr, args->powersupply.port);   
    } else {
        speed_t baudrate = args->powersupply.baudrate == 9600 ? B9600 : B115200;
        ps_fd = serial_open(args->powersupply.devpath, baudrate); 
    }

    if(ps_fd == -1) {
        close(ps_fd);
        fprintf(stderr, "Error while connecting to power supply: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
/*     3D PRINTER PART */
    if(args->printer_uses_inet) {
        printer_fd = tcp_connect(args->printer.ipaddr, args->printer.port);   
    } else {
        speed_t baudrate = args->printer.baudrate == 9600 ? B9600 : B115200;
        printer_fd = serial_open(args->printer.devpath, baudrate); 
    }

    if(printer_fd == -1) {
        close(printer_fd);
        fprintf(stderr, "Error while connecting to 3D printer: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}

static void sigint_handler(int sig) {
    if(ps_fd != -1)
        close(ps_fd);
    if(mm_fd != -1)
        close(mm_fd);
    if(printer_fd != -1)
        close(printer_fd);

}

static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    struct prog_args *arguments = state->input;
    
    switch (key)
    {
        case 'c': 
            arguments->config_path = arg; 
            break; 
        case 'p':
            arguments->powersupply.devpath = arg;
            arguments->ps_uses_inet = 0;
            break;
        case 'b':
            arguments->powersupply.baudrate = atoi(arg);
            break;
        case 'P': 
            arguments->powersupply.ipaddr = arg;
            arguments->ps_uses_inet = 1;
            break;
        case 'o':
            arguments->powersupply.port = atoi(arg);
            break;
        case 'm':
            arguments->multimeter.devpath = arg;
            arguments->mm_uses_inet = 0;
            break;
        case 'B':
            arguments->multimeter.baudrate = atoi(arg);
            break;
        case 'M':
            arguments->multimeter.ipaddr = arg;
            arguments->mm_uses_inet = 1;
            break;
        case 'O':
            arguments->multimeter.port = atoi(arg); // dlaczego  zmiana kolejności wpłynęła naz miane ??
        case 'd':
            arguments->printer.devpath = arg;
            arguments->printer_uses_inet = 0;
        case 'w':
            arguments->printer.baudrate = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            break;
        case ARGP_KEY_END:
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return EXIT_SUCCESS;
}

static void* receiver_thread(void* arg)
{
    if(arg == NULL) {
        fprintf(stderr, "Error: NULL argument passed to receiver thread, terminating");
        kill(getpid(), SIGTERM); 
    }

    int fd = ((struct recvt_args*)arg)->fd;
    struct ring_buffer_t* rb = ((struct recvt_args*)arg)->rb;

    char c;
    while(1) {
        if(read(fd, &c, 1) != -1) {
            rb_push(rb, c); 
        }
    }

    return NULL;
}
