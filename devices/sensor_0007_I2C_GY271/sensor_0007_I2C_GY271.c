
#include "trace.h"
#include "esp_err.h"
#include "math.h"
#include "ezlopi_i2c_master.h"
#include "sensor_0007_I2C_GY271.h"
#include "ezlopi_devices_list.h"
#include "ezlopi_timer.h"
#include "cJSON.h"
#include "ezlopi_cloud_category_str.h"
#include "ezlopi_item_name_str.h"
#include "ezlopi_cloud_subcategory_str.h"
#include "ezlopi_cloud_device_types_str.h"
#include "ezlopi_cloud_value_type_str.h"
#include "ezlopi_device_value_updated.h"
/*************************************************************************************************/
/*                              DEFINES                                                     */
/*************************************************************************************************/
static bool calibration_complete = false;

// static int calibrationData[3][2] = {{-32767, 32768},  // xmin,xmax
//                                     {-32767, 32768},  // ymin,ymax
//                                     {-32767, 32768}}; // zmin,zmax// Initialization added!
static int calibrationData[3][2] = {0};

static long bias_axis[3] = {0, 0, 0};        // (max_ + min_)/2
static long delta_axis[3] = {0, 0, 0};       // (max_ - min_)/2
static float delta_avg = 0;                  // (Delta_axis[0] + Delta_axis[1] + Delta_axis[2])/3
static float scale_axis[3] = {0, 0, 0};      // delta_avg / delta_axis
static float calibrated_axis[3] = {0, 0, 0}; // scale_axis[0] * ( raw_axis - bias_axis[0] )

#define REG_COUNT_LEN 6 // magnetometer data is to be read in one go .

//------------------------------------------------------------------------------
#define ADD_PROPERTIES_DEVICE_LIST(device_id, category, subcategory, item_name, value_type, cjson_device)                     \
    {                                                                                                                         \
        s_ezlopi_device_properties_t *_properties = sensor_i2c_gy271_prepare_properties(device_id, category, subcategory,     \
                                                                                        item_name, value_type, cjson_device); \
        if (NULL != _properties)                                                                                              \
        {                                                                                                                     \
            add_device_to_list(prep_arg, _properties, NULL);                                                                  \
        }                                                                                                                     \
    }

//------------------------------------------------------------------------------

static int sensor_i2c_gy271_prepare(void *arg);
static s_ezlopi_device_properties_t *sensor_i2c_gy271_prepare_properties(uint32_t DEVICE_ID, const char *CATEGORY, const char *SUB_CATEGORY, const char *ITEM_NAME, const char *VALUE_TYPE, cJSON *cjson_device);
static int add_device_to_list(s_ezlopi_prep_arg_t *prep_arg, s_ezlopi_device_properties_t *sensor_bme_device_properties, void *user_arg);
static int sensor_i2c_gy271_init(s_ezlopi_device_properties_t *properties, void *user_arg);
static int sensor_i2c_gy271_get_value_cjson(s_ezlopi_device_properties_t *properties, void *args);
static int sensor_i2c_gy271_configure_device(s_ezlopi_device_properties_t *properties, void *args);

static esp_err_t activate_set_reset_period(s_ezlopi_device_properties_t *properties);
static esp_err_t set_to_measure_mode(s_ezlopi_device_properties_t *properties);
static esp_err_t enable_data_ready_interrupt(s_ezlopi_device_properties_t *properties);

static esp_err_t gy271_check_data_ready_INTR(s_ezlopi_device_properties_t *properties, uint8_t *temp);
static void Correct_gy271_data(gy271_raw_data_t *RAW_DATA, gy271_data_t *data_p);
static void get_gy271_sensor_value(s_ezlopi_device_properties_t *properties, gy271_data_t *data_p);

void Gathering_initial_raw_max_min_values(s_ezlopi_device_properties_t *properties)
{
    //------------------------------------------------------------------------------
    int x = 0, y = 0, z = 0;
    uint8_t buffer_0, buffer_1;               // tempr
    uint8_t cal_tmp_buf[REG_COUNT_LEN] = {0}; // axis
    uint8_t Check_Register;
    uint8_t address_val;
    esp_err_t err = ESP_OK;
    if ((err = gy271_check_data_ready_INTR(properties, &Check_Register)) == ESP_OK)
    {
        // if 'bit0' in INTR register is set ; then read procced to read :- magnetometer registers
        if (Check_Register == GY271_DATA_SKIP_FLAG)
        {
            TRACE_E(" 1. FIRST_INIT_CALIB :--- Check_reg_val : {%x}", Check_Register);
            TRACE_E(" 1. FIRST_INIT_CALIB :--*********- DOR bit set..**********...");
            address_val = (GY271_DATA_Z_LSB_REGISTER);
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_0), 1);
            address_val = (GY271_DATA_Z_MSB_REGISTER);
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_1), 1);

            address_val = GY271_STATUS_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&Check_Register), 1);
            TRACE_W(" 1.{%x}", Check_Register);
            TRACE_E(" 1. FIRST_INIT_CALIB :--*********- DOR bit set..**********...");
        }
        if (Check_Register & GY271_DATA_READY_FLAG)
        {
            TRACE_W(" 1. FIRST_INIT_CALIB :--- Check_reg_val @ 0x00H: {%x}", Check_Register);
            TRACE_I(" 1. FIRST_INIT_CALIB :--- 00H reading started....");
            // read the axis data
            for (uint8_t i = 0; i < (REG_COUNT_LEN); i += 2)
            {
                // GY271_DATA_X_LSB_REGISTER = 0x00
                address_val = (GY271_DATA_X_LSB_REGISTER + i);
                ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
                ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (cal_tmp_buf + i), 1);
                // GY271_DATA_X_LSB_REGISTER = 0x00 +1
                address_val = (GY271_DATA_X_LSB_REGISTER + i + 1);
                ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
                ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (cal_tmp_buf + i + 1), 1);
            }
            // now read the status byte 0x06H
            address_val = GY271_STATUS_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&Check_Register), 1);
            TRACE_I(" 1. FIRST_INIT_CALIB :--- 06H reading ended....");
            TRACE_W(" 1. FIRST_INIT_CALIB :--- Check_reg_val @ 0x06H : {%x}", Check_Register);
        }
    }
    else
    {
        TRACE_E("Data not ready @reg{%x}... Error type:- %d ", (GY271_DATA_X_LSB_REGISTER), err);
    }

    // generate the raw_axis_values
    // Configure data structure // total 8 bytes
    x = (int16_t)(cal_tmp_buf[1] << 8 | cal_tmp_buf[0]); // x_axis =  0x01 [msb]  & 0x00 [lsb]
    y = (int16_t)(cal_tmp_buf[3] << 8 | cal_tmp_buf[2]); // y_axis =  0x03        & 0x02
    z = (int16_t)(cal_tmp_buf[5] << 8 | cal_tmp_buf[4]); // z_axis =  0x05        & 0x04

    //------------------------------------------------------------------------------
    if (x < calibrationData[0][0]) // xmin
    {
        calibrationData[0][0] = x;
    }
    if (x > calibrationData[0][1]) // xmax
    {
        calibrationData[0][1] = x;
    }
    if (y < calibrationData[1][0]) // ymin
    {
        calibrationData[1][0] = y;
    }
    if (y > calibrationData[1][1]) // ymax
    {
        calibrationData[1][1] = y;
    }
    if (z < calibrationData[2][0]) // zmin
    {
        calibrationData[2][0] = z;
    }
    if (z > calibrationData[2][1]) // zmax
    {
        calibrationData[2][1] = z;
    }
    //------------------------------------------------------------------------------
    TRACE_B("Calibrated :--- Xmin=%6d | Xmax=%6d | Ymin=%6d | Ymax=%6d | Zmin=%6d | Zmax=%6d ",
            calibrationData[0][0],
            calibrationData[0][1],
            calibrationData[1][0],
            calibrationData[1][1],
            calibrationData[2][0],
            calibrationData[2][1]);
    //------------------------------------------------------------------------------
}

void Gather_GY271_Calibration_data(void *params)
{
    s_ezlopi_device_properties_t *properties = (s_ezlopi_device_properties_t *)params;
    vTaskDelay(400); // 4  sec
    for (uint16_t i = 0; i <= 100; i++)
    {
        // call the data gathering function [100 * 20 ms = 20 sec]
        Gathering_initial_raw_max_min_values(properties);
        if (i % 5 == 0)
        {
            TRACE_I(" ------------------------------------------------- Time :- {%u sec} ", (i / 5));
        }
        vTaskDelay(20); // 200ms
    }

    TRACE_W("..........................Calculating Paramter.......................");
    // Calculate the : 1.bias_axis , 2.delta_axis , 3.delta_avg , 4.scale_axis
    // 1. bias_axis{x,y,z}
    bias_axis[0] = ((long)(calibrationData[0][1] + calibrationData[0][0]) / 2); // x-axis
    bias_axis[1] = ((long)(calibrationData[1][1] + calibrationData[1][0]) / 2); // y-axis
    bias_axis[2] = ((long)(calibrationData[2][1] + calibrationData[2][0]) / 2); // z-axis

    // 2. delta_axis{x,y,z}
    delta_axis[0] = (long)(calibrationData[0][1] - calibrationData[0][0]); // x-axis
    delta_axis[1] = (long)(calibrationData[1][1] - calibrationData[1][0]); // y-axis
    delta_axis[2] = (long)(calibrationData[2][1] - calibrationData[2][0]); // z-axis

    // 3. delta_avg
    delta_avg = (float)(delta_axis[0] + delta_axis[1] + delta_axis[2]) / 3.0f;

    // 4. Scale_axis{x,y,z}
    scale_axis[0] = delta_avg / delta_axis[0]; // x-axis
    scale_axis[1] = delta_avg / delta_axis[1]; // y-axis
    scale_axis[2] = delta_avg / delta_axis[2]; // z-axis

    ESP_LOGI("CAL_PARA", "Bias :--- _Xaxis=%6ld | _Yaxis=%6ld | _Zaxis=%6ld ",
             bias_axis[0],
             bias_axis[1],
             bias_axis[2]);

    ESP_LOGI("CAL_PARA", "Delta :--- _Xaxis=%6ld | _Yaxis=%6ld | _Zaxis=%6ld ",
             delta_axis[0],
             delta_axis[1],
             delta_axis[2]);
    ESP_LOGI("CAL_PARA", "Delta_AVG :--- %6f", delta_avg);

    ESP_LOGI("CAL_PARA", "Scale :--- _Xaxis=%6f | _Yaxis=%6f | _Zaxis=%6f ",
             scale_axis[0],
             scale_axis[1],
             scale_axis[0]);
    TRACE_W("...............................................................\n\n");

    calibration_complete = true;
    vTaskDelete(NULL);
}

//------------------------------------------------------------------------------

int sensor_0007_I2C_GY271(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg)
{

    switch (action)
    {
    case EZLOPI_ACTION_PREPARE:
    {
        sensor_i2c_gy271_prepare(arg);
        break;
    }
    case EZLOPI_ACTION_INITIALIZE:
    {
        sensor_i2c_gy271_init(properties, user_arg);
        break;
    }
    case EZLOPI_ACTION_GET_EZLOPI_VALUE:
    {
        sensor_i2c_gy271_get_value_cjson(properties, arg);
        break;
    }
    case EZLOPI_ACTION_NOTIFY_1000_MS:
    {
        static uint8_t count = 5;
        if (count++ > 5)
        {
            count = 0;
            if (calibration_complete)
            {
                TRACE_B("..................Calibration Complete.................");
                ezlopi_device_value_updated_from_device(properties);
            }
            else
            {
                TRACE_B("..................Calibrating.................\n");
            }
        }

        break;
    }
    default:
    {
        break;
    }
    }
    return 0;
}

//------------------------------------------------------------------------------

static int sensor_i2c_gy271_prepare(void *arg)
{
    int ret = 0;
    s_ezlopi_prep_arg_t *prep_arg = (s_ezlopi_prep_arg_t *)arg;

    if ((NULL != prep_arg) && (NULL != prep_arg->cjson_device))
    {
        uint32_t device_id = ezlopi_cloud_generate_device_id();
        ADD_PROPERTIES_DEVICE_LIST(device_id, category_generic_sensor, subcategory_not_defined, ezlopi_item_name_acceleration_x_axis, value_type_int, prep_arg->cjson_device);
        device_id = ezlopi_cloud_generate_device_id();
        ADD_PROPERTIES_DEVICE_LIST(device_id, category_generic_sensor, subcategory_not_defined, ezlopi_item_name_acceleration_y_axis, value_type_int, prep_arg->cjson_device);
        device_id = ezlopi_cloud_generate_device_id();
        ADD_PROPERTIES_DEVICE_LIST(device_id, category_generic_sensor, subcategory_not_defined, ezlopi_item_name_acceleration_z_axis, value_type_int, prep_arg->cjson_device);

        device_id = ezlopi_cloud_generate_device_id();
        ADD_PROPERTIES_DEVICE_LIST(device_id, category_temperature, subcategory_not_defined, ezlopi_item_name_temp, value_type_temperature, prep_arg->cjson_device);
    }
    return ret;
}

static int add_device_to_list(s_ezlopi_prep_arg_t *prep_arg, s_ezlopi_device_properties_t *sensor_GY271_device_properties, void *user_arg)
{
    int ret = 0;
    if (sensor_GY271_device_properties)
    {
        if (0 == ezlopi_devices_list_add(prep_arg->device, sensor_GY271_device_properties, user_arg))
        {
            free(sensor_GY271_device_properties);
        }
        else
        {
            ret = 1;
        }
    }
    return ret;
}

static s_ezlopi_device_properties_t *sensor_i2c_gy271_prepare_properties(uint32_t DEVICE_ID, const char *CATEGORY, const char *SUB_CATEGORY, const char *ITEM_NAME, const char *VALUE_TYPE, cJSON *cjson_device)
{
    s_ezlopi_device_properties_t *sensor_i2c_gy271_properties = NULL;

    if ((NULL != cjson_device))
    {
        sensor_i2c_gy271_properties = malloc(sizeof(s_ezlopi_device_properties_t));
        if (sensor_i2c_gy271_properties)
        {
            memset(sensor_i2c_gy271_properties, 0, sizeof(s_ezlopi_device_properties_t));
            sensor_i2c_gy271_properties->interface_type = EZLOPI_DEVICE_INTERFACE_I2C_MASTER;

            char *device_name = NULL;
            CJSON_GET_VALUE_STRING(cjson_device, "dev_name", device_name);
            ASSIGN_DEVICE_NAME(sensor_i2c_gy271_properties, device_name);
            sensor_i2c_gy271_properties->ezlopi_cloud.category = CATEGORY;
            sensor_i2c_gy271_properties->ezlopi_cloud.subcategory = SUB_CATEGORY;
            sensor_i2c_gy271_properties->ezlopi_cloud.item_name = ITEM_NAME;
            sensor_i2c_gy271_properties->ezlopi_cloud.device_type = dev_type_sensor;
            sensor_i2c_gy271_properties->ezlopi_cloud.value_type = VALUE_TYPE;
            sensor_i2c_gy271_properties->ezlopi_cloud.has_getter = true;
            sensor_i2c_gy271_properties->ezlopi_cloud.has_setter = false;
            sensor_i2c_gy271_properties->ezlopi_cloud.reachable = true;
            sensor_i2c_gy271_properties->ezlopi_cloud.battery_powered = false;
            sensor_i2c_gy271_properties->ezlopi_cloud.show = true;
            sensor_i2c_gy271_properties->ezlopi_cloud.room_name[0] = '\0';
            sensor_i2c_gy271_properties->ezlopi_cloud.device_id = DEVICE_ID;
            sensor_i2c_gy271_properties->ezlopi_cloud.room_id = ezlopi_cloud_generate_room_id();
            sensor_i2c_gy271_properties->ezlopi_cloud.item_id = ezlopi_cloud_generate_item_id();

            CJSON_GET_VALUE_INT(cjson_device, "gpio_scl", sensor_i2c_gy271_properties->interface.i2c_master.scl);
            CJSON_GET_VALUE_INT(cjson_device, "gpio_sda", sensor_i2c_gy271_properties->interface.i2c_master.sda);

            sensor_i2c_gy271_properties->interface.i2c_master.enable = true;
            sensor_i2c_gy271_properties->interface.i2c_master.clock_speed = 100000;
            sensor_i2c_gy271_properties->interface.i2c_master.address = GY271_ADDR;
        }
    }
    return sensor_i2c_gy271_properties;
}

static int sensor_i2c_gy271_init(s_ezlopi_device_properties_t *properties, void *user_arg)
{
    static bool guard = false;
    if (!guard)
    {
        guard = true;
        ezlopi_i2c_master_init(&properties->interface.i2c_master);

        TRACE_I("I2C initialized to channel %d", properties->interface.i2c_master.channel);
        sensor_i2c_gy271_configure_device(properties, user_arg);

        TRACE_W("Gathering Max-Min values for 2min.....");
        xTaskCreate(&Gather_GY271_Calibration_data, "GY271_Calibration_Task", 2048, properties, 1, NULL);
    }
    return (int)guard;
}

static int sensor_i2c_gy271_configure_device(s_ezlopi_device_properties_t *properties, void *args)
{
    int ret = 0;
    vTaskDelay(10); // 100 ms
    ESP_ERROR_CHECK_WITHOUT_ABORT(activate_set_reset_period(properties));
    ESP_ERROR_CHECK_WITHOUT_ABORT(set_to_measure_mode(properties));
    ESP_ERROR_CHECK_WITHOUT_ABORT(enable_data_ready_interrupt(properties));
    return ret;
}

//------------------------------------------------------------------------------

static esp_err_t activate_set_reset_period(s_ezlopi_device_properties_t *properties)
{
    uint8_t write_buffer[] = {GY271_SET_RESET_PERIOD_REGISTER, GY271_DEFAULT_SET_RESET_PERIOD}; // REG_INTR_STATUS;
    ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, write_buffer, 2);
    vTaskDelay(10);
    return ESP_OK;
}

static esp_err_t set_to_measure_mode(s_ezlopi_device_properties_t *properties)
{
    uint8_t write_byte[] = {GY271_CONTROL_REGISTER_1, GY271_OPERATION_MODE};
    ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, write_byte, 2);
    vTaskDelay(10);
    return ESP_OK;
}
static esp_err_t enable_data_ready_interrupt(s_ezlopi_device_properties_t *properties)
{
    uint8_t write_byte[] = {GY271_CONTROL_REGISTER_2, GY271_INT_EN};
    ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, write_byte, 2);
    vTaskDelay(10);
    return ESP_OK;
}

//------------------------------------------------------------------------------

static int sensor_i2c_gy271_get_value_cjson(s_ezlopi_device_properties_t *properties, void *args)
{
    int ret = 0;
    cJSON *cjson_properties = (cJSON *)args;
    float Data_value;
    gy271_data_t data_val = {0};

    // call a function to read all register data store it in a structure
    get_gy271_sensor_value(properties, &data_val);

    if (cjson_properties)
    {
        if (ezlopi_item_name_acceleration_x_axis == properties->ezlopi_cloud.item_name)
        {
            Data_value = (data_val.X);
            TRACE_I("X-axis : %.2f", Data_value);
            cJSON_AddNumberToObject(cjson_properties, "value", Data_value);
            cJSON_AddStringToObject(cjson_properties, "scale", "meter_per_square_second");
        }
        if (ezlopi_item_name_acceleration_y_axis == properties->ezlopi_cloud.item_name)
        {
            Data_value = (data_val.Y);
            TRACE_I("Y-axis : %.2f", Data_value);
            cJSON_AddNumberToObject(cjson_properties, "value", Data_value);
            cJSON_AddStringToObject(cjson_properties, "scale", "meter_per_square_second");
        }
        if (ezlopi_item_name_acceleration_z_axis == properties->ezlopi_cloud.item_name)
        {
            Data_value = (data_val.Z);
            TRACE_I("Z-axis : %.2f", Data_value);
            cJSON_AddNumberToObject(cjson_properties, "value", Data_value);
            cJSON_AddStringToObject(cjson_properties, "scale", "meter_per_square_second");
        }
        if (ezlopi_item_name_temp == properties->ezlopi_cloud.item_name)
        {
            Data_value = (data_val.T);
            TRACE_I("temperature : %.2f*C", Data_value);
            cJSON_AddNumberToObject(cjson_properties, "value", Data_value);
            cJSON_AddStringToObject(cjson_properties, "scale", "celsius");
        }

        ret = 1;
    }
    return ret;
}

//------------------------------------------------------------------------------

// function to check for INTR bit before any data extraction is done from acceleration registers
static esp_err_t gy271_check_data_ready_INTR(s_ezlopi_device_properties_t *properties, uint8_t *temp)
{
    esp_err_t err = ESP_OK;
    // Must request INTR_REG
    uint8_t write_buffer[] = {GY271_STATUS_REGISTER}; // REG_INTR_STATUS;
    ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, write_buffer, 1);
    // Read -> INTR_BIT
    ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, temp, 1);
    if (NULL != temp)
    {
        err = ESP_OK;
    }
    else
    {
        err = ESP_ERR_TIMEOUT;
    }
    return err;
}

static void reset_calib(void)
{
    calibrationData[0][0] = 0; //-32767; // xmin
    calibrationData[0][1] = 0; // 32768;  // xmax
    calibrationData[1][0] = 0; //-32767; // ymin
    calibrationData[1][1] = 0; // 32768;  // ymax
    calibrationData[2][0] = 0; //-32767; // zmin
    calibrationData[2][1] = 0; // 32768;  // zmax// Initialization added!
}

static int Get_azimuth(float dx, float dy)
{
    // calculate azimuth
    int azi = (int)((float)(180.0 * atan2(dy, dx)) / PI);
    return ((azi <= 0) ? (azi += 360) : (azi));
}

static void Correct_gy271_data(gy271_raw_data_t *RAW_DATA, gy271_data_t *data_p)
{
    // TRACE_B("x->%d", RAW_DATA->raw_x);
    // TRACE_B("y->%d", RAW_DATA->raw_y);
    // TRACE_B("z->%d", RAW_DATA->raw_z);
    // TRACE_B("T->%d *C", RAW_DATA->raw_temp);

    // update the calibration parameters
    calibrated_axis[0] = scale_axis[0] * (float)((long)RAW_DATA->raw_x - bias_axis[0]); // x-axis value
    calibrated_axis[1] = scale_axis[1] * (float)((long)RAW_DATA->raw_y - bias_axis[1]); // y-axis value
    calibrated_axis[2] = scale_axis[2] * (float)((long)RAW_DATA->raw_z - bias_axis[2]); // z-axis value

    // store the final data
    data_p->X = (calibrated_axis[0] / GY271_CONVERSION_TO_G);
    data_p->Y = (calibrated_axis[1] / GY271_CONVERSION_TO_G);
    data_p->Z = (calibrated_axis[2] / GY271_CONVERSION_TO_G);
    data_p->T = ((float)RAW_DATA->raw_temp / GY271_TEMPERATURE_SENSITIVITY) + 32.53f;
    // calculate azimuth
    data_p->azimuth = Get_azimuth((float)RAW_DATA->raw_x, (float)RAW_DATA->raw_y);

    TRACE_B("x->%.2f", data_p->X);
    TRACE_B("y->%.2f", data_p->Y);
    TRACE_B("z->%.2f", data_p->Z);
    TRACE_B("T->%.2f *C", data_p->T);
    TRACE_B("AZI->%d", data_p->azimuth);
}
static void get_gy271_sensor_value(s_ezlopi_device_properties_t *properties, gy271_data_t *data_ptr)
{
    static gy271_raw_data_t RAW_DATA = {0};
    static uint8_t tmp_buf[REG_COUNT_LEN] = {0}; // axis
    static uint8_t buffer_0, buffer_1;           // tempr
    uint8_t Check_Register;
    uint8_t address_val;
    esp_err_t err = ESP_OK;
    // Read specified FIFO buffer size (depends on configuration set)g

    if ((err = gy271_check_data_ready_INTR(properties, &Check_Register)) == ESP_OK)
    {

        // if 'bit0' in INTR register is set ; then read procced to read :- magnetometer registers
        if (Check_Register == GY271_DATA_SKIP_FLAG)
        {
            TRACE_E(" 2. Check_reg_val : {%x}", Check_Register);
            TRACE_E(" 2. ******* DOR bit set. *******....");
            address_val = (GY271_DATA_Z_LSB_REGISTER);
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_0), 1);
            address_val = (GY271_DATA_Z_MSB_REGISTER);
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_1), 1);

            address_val = GY271_STATUS_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&Check_Register), 1);
            TRACE_E(" 2. {%x}", Check_Register);
            TRACE_E(" 2.  ****** DOR bit set.n *******...");
        }
        if (Check_Register & GY271_DATA_READY_FLAG)
        {
            TRACE_W(" 2. Check_reg_val @ 0x00H: {%x}", Check_Register);
            TRACE_I(" 2. 00H reading started....");
            // read the axis data
            for (uint8_t i = 0; i < (REG_COUNT_LEN); i += 2)
            {
                // GY271_DATA_X_LSB_REGISTER = 0x00
                address_val = (GY271_DATA_X_LSB_REGISTER + i);
                ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
                ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (tmp_buf + i), 1);
                // GY271_DATA_X_LSB_REGISTER = 0x00 +1
                address_val = (GY271_DATA_X_LSB_REGISTER + i + 1);
                ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
                ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (tmp_buf + i + 1), 1);
            }

            // now read the status byte 0x06H
            address_val = GY271_STATUS_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&Check_Register), 1);
            TRACE_I(" 2. 06H reading ended....");
            TRACE_W(" 2. Check_reg_val @ 0x06H: {%x}", Check_Register);
            // read temperature data
            address_val = GY271_DATA_TEMP_LSB_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_0), 1);
            address_val = GY271_DATA_TEMP_MSB_REGISTER;
            ezlopi_i2c_master_write_to_device(&properties->interface.i2c_master, &address_val, 1);
            ezlopi_i2c_master_read_from_device(&properties->interface.i2c_master, (&buffer_1), 1);
        }
    }
    else
    {
        TRACE_E("Data not ready @reg{%x}... Error type:- %d ", (GY271_DATA_X_LSB_REGISTER), err);
    }
    // Configure data structure // total 8 bytes
    RAW_DATA.raw_x = (int16_t)(tmp_buf[1] << 8 | tmp_buf[0]);  // x_axis =  0x01 [msb]  & 0x00 [lsb]
    RAW_DATA.raw_y = (int16_t)(tmp_buf[3] << 8 | tmp_buf[2]);  // y_axis =  0x03        & 0x02
    RAW_DATA.raw_z = (int16_t)(tmp_buf[5] << 8 | tmp_buf[4]);  // z_axis =  0x05        & 0x04
    RAW_DATA.raw_temp = (int16_t)((buffer_1 << 8) | buffer_0); // tempr  =  buffer1     & buffer0
    Correct_gy271_data(&RAW_DATA, data_ptr);
}
