#define INTERVAL 15
#define MAXQUERY 500
#define BUF_LEN 80
#define WARMUP 15

#define DB "air"
#define TABLE "sen55"
#include <math.h>   // NAN
#include <stdio.h>  // printf
#include <time.h>  // printf

#include <mysql.h>

#include "sen5x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

int main(void) {
    time_t utime ;
    int16_t error = 0;
    unsigned long elapsed = 0 ;

    MYSQL *conn ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    char query[MAXQUERY] ;

    char *server = "localhost" ;
    char *user = "root" ;
    char *password = "R_250108_z" ;
    char *database = DB ;

    char tstampstring[BUF_LEN] ;

    conn = mysql_init(NULL) ;

    if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    /* send SQL query */
    if (mysql_query(conn, "show tables")) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    /* output table name */
    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) != NULL)
        printf("%s \n", row[0]);

    sensirion_i2c_hal_init();

    error = sen5x_device_reset();
    if (error) {
        printf("Error executing sen5x_device_reset(): %i\n", error);
    }

    unsigned char serial_number[32];
    uint8_t serial_number_size = 32;
    error = sen5x_get_serial_number(serial_number, serial_number_size);
    if (error) {
        printf("Error executing sen5x_get_serial_number(): %i\n", error);
    } else {
        printf("Serial number: %s\n", serial_number);
    }

    unsigned char product_name[32];
    uint8_t product_name_size = 32;
    error = sen5x_get_product_name(product_name, product_name_size);
    if (error) {
        printf("Error executing sen5x_get_product_name(): %i\n", error);
    } else {
        printf("Product name: %s\n", product_name);
    }

    uint8_t firmware_major;
    uint8_t firmware_minor;
    bool firmware_debug;
    uint8_t hardware_major;
    uint8_t hardware_minor;
    uint8_t protocol_major;
    uint8_t protocol_minor;
    error = sen5x_get_version(&firmware_major, &firmware_minor, &firmware_debug,
                              &hardware_major, &hardware_minor, &protocol_major,
                              &protocol_minor);
    if (error) {
        printf("Error executing sen5x_get_version(): %i\n", error);
    } else {
        printf("Firmware: %u.%u, Hardware: %u.%u\n", firmware_major,
               firmware_minor, hardware_major, hardware_minor);
    }

    // set a temperature offset in degrees celsius
    // Note: supported by SEN54 and SEN55 sensors
    // By default, the temperature and humidity outputs from the sensor
    // are compensated for the modules self-heating. If the module is
    // designed into a device, the temperature compensation might need
    // to be adapted to incorporate the change in thermal coupling and
    // self-heating of other device components.
    //
    // A guide to achieve optimal performance, including references
    // to mechanical design-in examples can be found in the app note
    // “SEN5x – Temperature Compensation Instruction” at www.sensirion.com.
    // Please refer to those application notes for further information
    // on the advanced compensation settings used in
    // `sen5x_set_temperature_offset_parameters`,
    // `sen5x_set_warm_start_parameter` and `sen5x_set_rht_acceleration_mode`.
    //
    // Adjust temp_offset to account for additional temperature offsets
    // exceeding the SEN module's self heating.
    float temp_offset = 0.0f;
    error = sen5x_set_temperature_offset_simple(temp_offset);
    if (error) {
        printf("Error executing sen5x_set_temperature_offset_simple(): %i\n",
               error);
    } else {
        printf("Temperature Offset set to %.2f °C (SEN54/SEN55 only)\n",
               temp_offset);
    }


    int16_t index_offset ;
    int16_t learning_time_offset_hours ;
    int16_t learning_time_gain_hours ;
    int16_t gating_max_duration_minutes ;
    int16_t std_initial ;
    int16_t gain_factor ;

    sen5x_get_nox_algorithm_tuning_parameters(&index_offset, &learning_time_offset_hours, &learning_time_gain_hours, &gating_max_duration_minutes, &std_initial, &gain_factor) ;
    printf("index_offset: %d\n", index_offset) ;
    printf("learning_time_offset_hours: %d\n", learning_time_offset_hours) ;
    printf("learning_time_gain_hours: %d\n", learning_time_gain_hours) ;
    printf("gating_max_duration_minutes: %d\n", gating_max_duration_minutes) ;
    printf("std_initial: %d\n", std_initial) ;
    printf("gain_factor: %d\n", gain_factor) ;

    index_offset = 1 ;
    sen5x_set_nox_algorithm_tuning_parameters(index_offset, learning_time_offset_hours, learning_time_gain_hours, gating_max_duration_minutes, std_initial, gain_factor) ;

    sen5x_get_voc_algorithm_tuning_parameters(&index_offset, &learning_time_offset_hours, &learning_time_gain_hours, &gating_max_duration_minutes, &std_initial, &gain_factor) ;
    index_offset = 1 ;
    sen5x_set_voc_algorithm_tuning_parameters(index_offset, learning_time_offset_hours, learning_time_gain_hours, gating_max_duration_minutes, std_initial, gain_factor) ;

    // Start Measurement
    error = sen5x_start_measurement();

    if (error) {
        printf("Error executing sen5x_start_measurement(): %i\n", error);
    }

    for (;;) {
	time(&utime) ;

	struct tm *ptm = localtime(&utime) ;
	strftime(tstampstring, BUF_LEN, "%Y-%m-%d %T", ptm) ;

        // Read Measurement
        sensirion_i2c_hal_sleep_usec(1000000);

        float mass_concentration_pm1p0;
        float mass_concentration_pm2p5;
        float mass_concentration_pm4p0;
        float mass_concentration_pm10p0;
        float ambient_humidity;
        float ambient_temperature;
        float voc_index;
        float nox_index;

        error = sen5x_read_measured_values(
            &mass_concentration_pm1p0, &mass_concentration_pm2p5,
            &mass_concentration_pm4p0, &mass_concentration_pm10p0,
            &ambient_humidity, &ambient_temperature, &voc_index, &nox_index);
        if (error) {
            printf("Error executing sen5x_read_measured_values(): %i\n", error);
        } else {
#if 0
            printf("Mass concentration pm1p0: %.1f µg/m³\n",
                   mass_concentration_pm1p0);
            printf("Mass concentration pm2p5: %.1f µg/m³\n",
                   mass_concentration_pm2p5);
            printf("Mass concentration pm4p0: %.1f µg/m³\n",
                   mass_concentration_pm4p0);
            printf("Mass concentration pm10p0: %.1f µg/m³\n",
                   mass_concentration_pm10p0);
            if (isnan(ambient_humidity)) {
                printf("Ambient humidity: n/a\n");
            } else {
                printf("Ambient humidity: %.1f %%RH\n", ambient_humidity);
            }
            if (isnan(ambient_temperature)) {
                printf("Ambient temperature: n/a\n");
            } else {
                printf("Ambient temperature: %.1f °C\n", ambient_temperature);
            }
            if (isnan(voc_index)) {
                printf("Voc index: n/a\n");
            } else {
                printf("Voc index: %.1f\n", voc_index);
            }
            if (isnan(nox_index)) {
                printf("Nox index: n/a\n");
            } else {
                printf("Nox index: %.1f\n", nox_index);
            }

#endif
            if (isnan(voc_index)) {
		voc_index = -1 ;
	    }
            if (isnan(nox_index)) {
		nox_index = -1 ;
	    }

	    if (elapsed > WARMUP && elapsed % INTERVAL == 0) {
	    	sprintf(query, "insert into %s (unixtime, timestamp, pm1p0, pm2p5, pm4p0, pm10p0, humidity, temperature, voc, nox) values (%lu, \"%s\", %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.0f, %.0f)", TABLE, utime, tstampstring, mass_concentration_pm1p0, mass_concentration_pm2p5, mass_concentration_pm4p0, mass_concentration_pm10p0, ambient_humidity, ambient_temperature, voc_index, nox_index);
	        if (mysql_query(conn, query)) {
		    fprintf(stderr, "%s\n", mysql_error(conn)) ;
		    exit(1) ;
	        }
	    }

	    elapsed += 1 ;
        }
    }

    error = sen5x_stop_measurement();
    if (error) {
        printf("Error executing sen5x_stop_measurement(): %i\n", error);
    }

    mysql_close(conn) ;

    return 0;
}
