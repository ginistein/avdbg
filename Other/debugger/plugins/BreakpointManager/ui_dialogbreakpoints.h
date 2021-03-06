/********************************************************************************
** Form generated from reading UI file 'dialogbreakpoints.ui'
**
** Created: Sat Sep 15 17:40:59 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGBREAKPOINTS_H
#define UI_DIALOGBREAKPOINTS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogBreakpoints
{
public:
    QGridLayout *gridLayout;
    QPushButton *btnCondition;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *btnRemove;
    QPushButton *btnAdd;
    QTableWidget *tableWidget;

    void setupUi(QDialog *DialogBreakpoints)
    {
        if (DialogBreakpoints->objectName().isEmpty())
            DialogBreakpoints->setObjectName(QString::fromUtf8("DialogBreakpoints"));
        DialogBreakpoints->resize(803, 264);
        gridLayout = new QGridLayout(DialogBreakpoints);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnCondition = new QPushButton(DialogBreakpoints);
        btnCondition->setObjectName(QString::fromUtf8("btnCondition"));

        gridLayout->addWidget(btnCondition, 2, 1, 1, 1);

        spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(spacerItem, 3, 1, 1, 1);

        okButton = new QPushButton(DialogBreakpoints);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setDefault(true);

        gridLayout->addWidget(okButton, 4, 1, 1, 1);

        btnRemove = new QPushButton(DialogBreakpoints);
        btnRemove->setObjectName(QString::fromUtf8("btnRemove"));

        gridLayout->addWidget(btnRemove, 1, 1, 1, 1);

        btnAdd = new QPushButton(DialogBreakpoints);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));

        gridLayout->addWidget(btnAdd, 0, 1, 1, 1);

        tableWidget = new QTableWidget(DialogBreakpoints);
        if (tableWidget->columnCount() < 5)
            tableWidget->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setAlternatingRowColors(true);
        tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidget->horizontalHeader()->setStretchLastSection(true);
        tableWidget->verticalHeader()->setVisible(false);

        gridLayout->addWidget(tableWidget, 0, 0, 5, 1);

        QWidget::setTabOrder(tableWidget, btnAdd);
        QWidget::setTabOrder(btnAdd, btnRemove);
        QWidget::setTabOrder(btnRemove, btnCondition);
        QWidget::setTabOrder(btnCondition, okButton);

        retranslateUi(DialogBreakpoints);
        QObject::connect(okButton, SIGNAL(clicked()), DialogBreakpoints, SLOT(accept()));

        QMetaObject::connectSlotsByName(DialogBreakpoints);
    } // setupUi

    void retranslateUi(QDialog *DialogBreakpoints)
    {
        DialogBreakpoints->setWindowTitle(QApplication::translate("DialogBreakpoints", "Breakpoint Manager", 0, QApplication::UnicodeUTF8));
        btnCondition->setText(QApplication::translate("DialogBreakpoints", "Set Breakpoint &Condition", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("DialogBreakpoints", "&Close", 0, QApplication::UnicodeUTF8));
        btnRemove->setText(QApplication::translate("DialogBreakpoints", "&Remove Breakpoint", 0, QApplication::UnicodeUTF8));
        btnAdd->setText(QApplication::translate("DialogBreakpoints", "&Add Breakpoint", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("DialogBreakpoints", "Address", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("DialogBreakpoints", "Condition", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("DialogBreakpoints", "Original Byte", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("DialogBreakpoints", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("DialogBreakpoints", "Function", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogBreakpoints: public Ui_DialogBreakpoints {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGBREAKPOINTS_H
