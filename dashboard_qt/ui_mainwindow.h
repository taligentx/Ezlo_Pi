/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionExit;
    QAction *actionAbout_EzloPi;
    QAction *actionRegister;
    QAction *actionLogin;
    QAction *actionClear_Table;
    QAction *actionDisable;
    QAction *actionInfo;
    QAction *actionDebug;
    QWidget *centralWidget;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_scan_uart_ports;
    QComboBox *comboBox_uart_list;
    QPushButton *pushButton_connect_uart;
    QPushButton *pushButton_set_wifi;
    QPushButton *pushButton_erase_flash;
    QPushButton *pushButton_flash_ezpi_bins;
    QPushButton *pushButton_clear_uart_direct_log;
    QSplitter *splitter;
    QTableWidget *tableWidget_device_table;
    QTextBrowser *textBrowser_console_log;
    QGroupBox *groupBox_device_config;
    QPushButton *pushButton_get_ezpi_config;
    QPushButton *pushButton_set_ezpi_config;
    QGroupBox *groupBox;
    QPushButton *pushButton_add_device;
    QPushButton *pushButton_remove_device;
    QComboBox *comboBox_esp32_board_type;
    QLabel *label_board_select;
    QLabel *label_registered_devices;
    QComboBox *comboBox_registered_devices;
    QGroupBox *groupBox_reset;
    QPushButton *pushButton_device_restart;
    QPushButton *pushButton_device_factory_reset;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QMenu *menuView;
    QMenu *menuConsole_Log_mode;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(810, 628);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/res/icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionAbout_EzloPi = new QAction(MainWindow);
        actionAbout_EzloPi->setObjectName(QString::fromUtf8("actionAbout_EzloPi"));
        actionRegister = new QAction(MainWindow);
        actionRegister->setObjectName(QString::fromUtf8("actionRegister"));
        actionLogin = new QAction(MainWindow);
        actionLogin->setObjectName(QString::fromUtf8("actionLogin"));
        actionClear_Table = new QAction(MainWindow);
        actionClear_Table->setObjectName(QString::fromUtf8("actionClear_Table"));
        actionDisable = new QAction(MainWindow);
        actionDisable->setObjectName(QString::fromUtf8("actionDisable"));
        actionDisable->setCheckable(true);
        actionInfo = new QAction(MainWindow);
        actionInfo->setObjectName(QString::fromUtf8("actionInfo"));
        actionInfo->setCheckable(true);
        actionInfo->setChecked(true);
        actionDebug = new QAction(MainWindow);
        actionDebug->setObjectName(QString::fromUtf8("actionDebug"));
        actionDebug->setCheckable(true);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 10, 781, 31));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pushButton_scan_uart_ports = new QPushButton(layoutWidget);
        pushButton_scan_uart_ports->setObjectName(QString::fromUtf8("pushButton_scan_uart_ports"));

        horizontalLayout->addWidget(pushButton_scan_uart_ports);

        comboBox_uart_list = new QComboBox(layoutWidget);
        comboBox_uart_list->setObjectName(QString::fromUtf8("comboBox_uart_list"));

        horizontalLayout->addWidget(comboBox_uart_list);

        pushButton_connect_uart = new QPushButton(layoutWidget);
        pushButton_connect_uart->setObjectName(QString::fromUtf8("pushButton_connect_uart"));

        horizontalLayout->addWidget(pushButton_connect_uart);

        pushButton_set_wifi = new QPushButton(layoutWidget);
        pushButton_set_wifi->setObjectName(QString::fromUtf8("pushButton_set_wifi"));

        horizontalLayout->addWidget(pushButton_set_wifi);

        pushButton_erase_flash = new QPushButton(layoutWidget);
        pushButton_erase_flash->setObjectName(QString::fromUtf8("pushButton_erase_flash"));

        horizontalLayout->addWidget(pushButton_erase_flash);

        pushButton_flash_ezpi_bins = new QPushButton(layoutWidget);
        pushButton_flash_ezpi_bins->setObjectName(QString::fromUtf8("pushButton_flash_ezpi_bins"));

        horizontalLayout->addWidget(pushButton_flash_ezpi_bins);

        pushButton_clear_uart_direct_log = new QPushButton(layoutWidget);
        pushButton_clear_uart_direct_log->setObjectName(QString::fromUtf8("pushButton_clear_uart_direct_log"));

        horizontalLayout->addWidget(pushButton_clear_uart_direct_log);

        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setGeometry(QRect(10, 60, 611, 471));
        splitter->setOrientation(Qt::Vertical);
        tableWidget_device_table = new QTableWidget(splitter);
        if (tableWidget_device_table->columnCount() < 4)
            tableWidget_device_table->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_device_table->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_device_table->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_device_table->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget_device_table->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        tableWidget_device_table->setObjectName(QString::fromUtf8("tableWidget_device_table"));
        tableWidget_device_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        splitter->addWidget(tableWidget_device_table);
        textBrowser_console_log = new QTextBrowser(splitter);
        textBrowser_console_log->setObjectName(QString::fromUtf8("textBrowser_console_log"));
        splitter->addWidget(textBrowser_console_log);
        groupBox_device_config = new QGroupBox(centralWidget);
        groupBox_device_config->setObjectName(QString::fromUtf8("groupBox_device_config"));
        groupBox_device_config->setGeometry(QRect(637, 425, 151, 101));
        pushButton_get_ezpi_config = new QPushButton(groupBox_device_config);
        pushButton_get_ezpi_config->setObjectName(QString::fromUtf8("pushButton_get_ezpi_config"));
        pushButton_get_ezpi_config->setGeometry(QRect(20, 24, 111, 31));
        pushButton_set_ezpi_config = new QPushButton(groupBox_device_config);
        pushButton_set_ezpi_config->setObjectName(QString::fromUtf8("pushButton_set_ezpi_config"));
        pushButton_set_ezpi_config->setGeometry(QRect(20, 64, 111, 31));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(639, 300, 149, 111));
        pushButton_add_device = new QPushButton(groupBox);
        pushButton_add_device->setObjectName(QString::fromUtf8("pushButton_add_device"));
        pushButton_add_device->setGeometry(QRect(20, 25, 111, 31));
        pushButton_remove_device = new QPushButton(groupBox);
        pushButton_remove_device->setObjectName(QString::fromUtf8("pushButton_remove_device"));
        pushButton_remove_device->setGeometry(QRect(20, 68, 111, 31));
        comboBox_esp32_board_type = new QComboBox(centralWidget);
        comboBox_esp32_board_type->addItem(QString());
        comboBox_esp32_board_type->addItem(QString());
        comboBox_esp32_board_type->addItem(QString());
        comboBox_esp32_board_type->setObjectName(QString::fromUtf8("comboBox_esp32_board_type"));
        comboBox_esp32_board_type->setGeometry(QRect(650, 254, 131, 26));
        label_board_select = new QLabel(centralWidget);
        label_board_select->setObjectName(QString::fromUtf8("label_board_select"));
        label_board_select->setGeometry(QRect(650, 229, 111, 20));
        label_registered_devices = new QLabel(centralWidget);
        label_registered_devices->setObjectName(QString::fromUtf8("label_registered_devices"));
        label_registered_devices->setGeometry(QRect(650, 169, 131, 20));
        comboBox_registered_devices = new QComboBox(centralWidget);
        comboBox_registered_devices->setObjectName(QString::fromUtf8("comboBox_registered_devices"));
        comboBox_registered_devices->setGeometry(QRect(650, 194, 131, 26));
        groupBox_reset = new QGroupBox(centralWidget);
        groupBox_reset->setObjectName(QString::fromUtf8("groupBox_reset"));
        groupBox_reset->setGeometry(QRect(641, 53, 141, 111));
        pushButton_device_restart = new QPushButton(groupBox_reset);
        pushButton_device_restart->setObjectName(QString::fromUtf8("pushButton_device_restart"));
        pushButton_device_restart->setGeometry(QRect(19, 23, 111, 29));
        pushButton_device_factory_reset = new QPushButton(groupBox_reset);
        pushButton_device_factory_reset->setObjectName(QString::fromUtf8("pushButton_device_factory_reset"));
        pushButton_device_factory_reset->setGeometry(QRect(17, 65, 121, 29));
        MainWindow->setCentralWidget(centralWidget);
        groupBox_reset->raise();
        layoutWidget->raise();
        splitter->raise();
        groupBox_device_config->raise();
        groupBox->raise();
        comboBox_esp32_board_type->raise();
        label_board_select->raise();
        label_registered_devices->raise();
        comboBox_registered_devices->raise();
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 810, 26));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuConsole_Log_mode = new QMenu(menuView);
        menuConsole_Log_mode->setObjectName(QString::fromUtf8("menuConsole_Log_mode"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionLogin);
        menuFile->addAction(actionRegister);
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionAbout_EzloPi);
        menuView->addAction(menuConsole_Log_mode->menuAction());
        menuView->addAction(actionClear_Table);
        menuConsole_Log_mode->addAction(actionDisable);
        menuConsole_Log_mode->addAction(actionInfo);
        menuConsole_Log_mode->addAction(actionDebug);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "EzLogic - EzloPi", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        actionAbout_EzloPi->setText(QCoreApplication::translate("MainWindow", "About EzloPi", nullptr));
        actionRegister->setText(QCoreApplication::translate("MainWindow", "Register", nullptr));
        actionLogin->setText(QCoreApplication::translate("MainWindow", "Login", nullptr));
        actionClear_Table->setText(QCoreApplication::translate("MainWindow", "Clear Table and data", nullptr));
        actionDisable->setText(QCoreApplication::translate("MainWindow", "Disable", nullptr));
        actionInfo->setText(QCoreApplication::translate("MainWindow", "Info", nullptr));
        actionDebug->setText(QCoreApplication::translate("MainWindow", "Debug", nullptr));
        pushButton_scan_uart_ports->setText(QCoreApplication::translate("MainWindow", "Scan", nullptr));
        pushButton_connect_uart->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        pushButton_set_wifi->setText(QCoreApplication::translate("MainWindow", "Configure WiFi", nullptr));
        pushButton_erase_flash->setText(QCoreApplication::translate("MainWindow", "Erase Device", nullptr));
        pushButton_flash_ezpi_bins->setText(QCoreApplication::translate("MainWindow", "Flash Firmware", nullptr));
        pushButton_clear_uart_direct_log->setText(QCoreApplication::translate("MainWindow", "Clear Logs", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_device_table->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Device Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_device_table->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_device_table->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Sub type", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_device_table->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "GPIOs", nullptr));
        textBrowser_console_log->setPlaceholderText(QCoreApplication::translate("MainWindow", "log console", nullptr));
        groupBox_device_config->setTitle(QCoreApplication::translate("MainWindow", "Configure", nullptr));
        pushButton_get_ezpi_config->setText(QCoreApplication::translate("MainWindow", "Get Config", nullptr));
        pushButton_set_ezpi_config->setText(QCoreApplication::translate("MainWindow", "Set Config", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Device", nullptr));
        pushButton_add_device->setText(QCoreApplication::translate("MainWindow", "Add Device", nullptr));
        pushButton_remove_device->setText(QCoreApplication::translate("MainWindow", "Remove Device", nullptr));
        comboBox_esp32_board_type->setItemText(0, QCoreApplication::translate("MainWindow", "ESP32 Generic", nullptr));
        comboBox_esp32_board_type->setItemText(1, QCoreApplication::translate("MainWindow", "ESP32C3", nullptr));
        comboBox_esp32_board_type->setItemText(2, QCoreApplication::translate("MainWindow", "ESP32S3", nullptr));

        label_board_select->setText(QCoreApplication::translate("MainWindow", "Board Select", nullptr));
        label_registered_devices->setText(QCoreApplication::translate("MainWindow", "Registered devices", nullptr));
        groupBox_reset->setTitle(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        pushButton_device_restart->setText(QCoreApplication::translate("MainWindow", "Restart", nullptr));
        pushButton_device_factory_reset->setText(QCoreApplication::translate("MainWindow", "Factory Restore", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuConsole_Log_mode->setTitle(QCoreApplication::translate("MainWindow", "Console Log mode", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
