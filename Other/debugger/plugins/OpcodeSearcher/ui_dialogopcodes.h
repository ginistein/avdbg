/********************************************************************************
** Form generated from reading UI file 'dialogopcodes.ui'
**
** Created: Sat Sep 15 17:41:31 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGOPCODES_H
#define UI_DIALOGOPCODES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableView>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogOpcodes
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QGroupBox *groupBox;
    QVBoxLayout *vboxLayout;
    QRadioButton *radioButton;
    QComboBox *comboBox;
    QSpacerItem *spacerItem;
    QLabel *label_3;
    QLineEdit *txtSearch;
    QTableView *tableView;
    QLabel *label_2;
    QListWidget *listWidget;
    QHBoxLayout *hboxLayout;
    QPushButton *btnClose;
    QPushButton *btnHelp;
    QSpacerItem *spacerItem1;
    QPushButton *btnFind;
    QProgressBar *progressBar;

    void setupUi(QDialog *DialogOpcodes)
    {
        if (DialogOpcodes->objectName().isEmpty())
            DialogOpcodes->setObjectName(QString::fromUtf8("DialogOpcodes"));
        DialogOpcodes->resize(691, 456);
        gridLayout = new QGridLayout(DialogOpcodes);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(DialogOpcodes);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 2);

        groupBox = new QGroupBox(DialogOpcodes);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        vboxLayout = new QVBoxLayout(groupBox);
        vboxLayout->setSpacing(6);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        radioButton = new QRadioButton(groupBox);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));
        radioButton->setChecked(true);

        vboxLayout->addWidget(radioButton);

        comboBox = new QComboBox(groupBox);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));

        vboxLayout->addWidget(comboBox);

        spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem);


        gridLayout->addWidget(groupBox, 0, 2, 3, 1);

        label_3 = new QLabel(DialogOpcodes);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        txtSearch = new QLineEdit(DialogOpcodes);
        txtSearch->setObjectName(QString::fromUtf8("txtSearch"));

        gridLayout->addWidget(txtSearch, 1, 1, 1, 1);

        tableView = new QTableView(DialogOpcodes);
        tableView->setObjectName(QString::fromUtf8("tableView"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        tableView->setFont(font);
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setSortingEnabled(true);
        tableView->horizontalHeader()->setStretchLastSection(true);

        gridLayout->addWidget(tableView, 2, 0, 1, 2);

        label_2 = new QLabel(DialogOpcodes);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        listWidget = new QListWidget(DialogOpcodes);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setFont(font);
        listWidget->setAlternatingRowColors(true);

        gridLayout->addWidget(listWidget, 4, 0, 1, 3);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        btnClose = new QPushButton(DialogOpcodes);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        hboxLayout->addWidget(btnClose);

        btnHelp = new QPushButton(DialogOpcodes);
        btnHelp->setObjectName(QString::fromUtf8("btnHelp"));
        btnHelp->setEnabled(false);

        hboxLayout->addWidget(btnHelp);

        spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem1);

        btnFind = new QPushButton(DialogOpcodes);
        btnFind->setObjectName(QString::fromUtf8("btnFind"));
        btnFind->setDefault(true);

        hboxLayout->addWidget(btnFind);


        gridLayout->addLayout(hboxLayout, 5, 0, 1, 3);

        progressBar = new QProgressBar(DialogOpcodes);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(0);
        progressBar->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(progressBar, 6, 0, 1, 3);

        QWidget::setTabOrder(txtSearch, tableView);
        QWidget::setTabOrder(tableView, radioButton);
        QWidget::setTabOrder(radioButton, comboBox);
        QWidget::setTabOrder(comboBox, listWidget);
        QWidget::setTabOrder(listWidget, btnClose);
        QWidget::setTabOrder(btnClose, btnHelp);
        QWidget::setTabOrder(btnHelp, btnFind);

        retranslateUi(DialogOpcodes);
        QObject::connect(btnClose, SIGNAL(clicked()), DialogOpcodes, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogOpcodes);
    } // setupUi

    void retranslateUi(QDialog *DialogOpcodes)
    {
        DialogOpcodes->setWindowTitle(QApplication::translate("DialogOpcodes", "Opcode Search", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DialogOpcodes", "Regions To Search:", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("DialogOpcodes", "What To Search For", 0, QApplication::UnicodeUTF8));
        radioButton->setText(QApplication::translate("DialogOpcodes", "&Jump Equivalent", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DialogOpcodes", "Filter", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DialogOpcodes", "Results:", 0, QApplication::UnicodeUTF8));
        btnClose->setText(QApplication::translate("DialogOpcodes", "&Close", 0, QApplication::UnicodeUTF8));
        btnHelp->setText(QApplication::translate("DialogOpcodes", "&Help", 0, QApplication::UnicodeUTF8));
        btnFind->setText(QApplication::translate("DialogOpcodes", "&Find", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogOpcodes: public Ui_DialogOpcodes {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGOPCODES_H
