#ifndef _CMDS_H
#define _CMDS_H 

int scpi_cmd(int fd, const char* cmd_format, ...);

// macros for power supply commands. For more: https://sigrok.org/wiki/Korad_KAxxxxP_series
#define PS_SET_VOLT(voltage) scpi_cmd(ps_fd, "VSET1:%.2f", voltage)
#define PS_SET_CURR(current) scpi_cmd(ps_fd, "ISET1:%.2f", current)
#define PS_ENABLE_OUT()     scpi_cmd(ps_fd, "OUT1")
#define PS_DISABLE_OUT()    scpi_cmd(ps_fd, "OUT0")

// macros for multimeter commands. For more: https://int.siglent.com/upload_file/user/SDM3055/SDM3055_RemoteManual_RC06035-E01A.pdf
#define MEAS_VOLT_RANGE(type, range) scpi_cmd(mm_fd, "MEAS:VOLT:%s? %s\r\n", type, range)
#define MEAS_VOLT_DC_RANGE(range) MEAS_VOLT_RANGE("DC", range)
#define MEAS_VOLT_AC_RANGE(range) MEAS_VOLT_RANGE("AC", range)
#define MEAS_SET_OPT_FILTER() scpi_cmd(mm_fd, "VOLT:FILTER ON\r\n")

// macros for 3d printer. For more: https://marlinfw.org/meta/gcode/
#define PRINT_MOVE_G1_X(value) scpi_cmd(printer_fd, "G1 X%.1f\n", value)
#define PRINT_MOVE_G1_Z(value) scpi_cmd(printer_fd, "G1 Z%.1f\n", value)
#define PRINT_MOVE_G1_Y(value) scpi_cmd(printer_fd, "G1 Y%.1f\n", value)


#endif /* _CMDS_H */ 
