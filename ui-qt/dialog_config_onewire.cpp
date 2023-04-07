#include "dialog_config_onewire.h"
#include "ui_dialog_config_onewire.h"

Dialog_config_onewire::Dialog_config_onewire(QWidget *parent, EzPi * EzloPi) :
    QDialog(parent),
    ui(new Ui::Dialog_config_onewire)
{
    ui->setupUi(this);

    ezloPi_one_wire = EzloPi;

    std::vector<EZPI_UINT8> gpio_pool = ezloPi_one_wire->EZPI_GET_GPIO_POOL();
    EZPI_UINT8 gpio_pool_count = (EZPI_UINT8)gpio_pool.size();

    for(EZPI_UINT8 i = 0; i < gpio_pool_count; i++) {
        if(gpio_pool[i] == EZPI_DEV_TYPE_UNCONFIGURED)
            ui->comboBox_onewire_gpio->addItem(QString::number(i));
    }

    ui->lineEdit_device_name->setText(ezloPi_one_wire->EZPI_GET_DEV_TYPE(EZPI_DEV_TYPE_ONE_WIRE) + \
                                      " " + QString::number(ezloPi_one_wire->EZPI_GET_ONEWIRE_DEVICES().size() + 1));
}

Dialog_config_onewire::~Dialog_config_onewire() {
    delete ui;
}

void Dialog_config_onewire::on_buttonBox_accepted() {
    ezpi_device_one_wire_t onewire_user_data;
    onewire_user_data.dev_type = EZPI_DEV_TYPE_ONE_WIRE;
    onewire_user_data.dev_name = ui->lineEdit_device_name->text();
    onewire_user_data.id_room = ""; //TBD

    if(ui->comboBox_onewire_subtype->currentIndex() == 0) {
        onewire_user_data.id_item = EZPI_ITEM_TYPE_DHT22;
    } else if(ui->comboBox_onewire_subtype->currentIndex() == 1) {
        onewire_user_data.id_item = EZPI_ITEM_TYPE_DS18B20;
    } else {

    }

    onewire_user_data.gpio = ui->comboBox_onewire_gpio->currentText().toInt();

    // Adding device to the device vector
    if(ezloPi_one_wire->EZPI_ADD_ONEWIRE_DEVICE(onewire_user_data) == EZPI_SUCCESS) {
//       QMessageBox::information(this, "Success", "Successfully added a one wire device.");
       // Trigger signal to add device in the table
       emit ezpi_signal_dev_onewire_added(EZPI_DEV_TYPE_ONE_WIRE);
    } else if(ezloPi_one_wire->EZPI_ADD_ONEWIRE_DEVICE(onewire_user_data) == EZPI_ERROR_REACHED_MAX_DEV) {
       QMessageBox::information(this, "Error", "Error : Reached maximum one wire device limit.");
    } else {
        // Do nothing
    }
}

