/********************************************************************************
** Form generated from reading UI file 'dialogasciistring.ui'
**
** Created: Sat Sep 15 17:40:55 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGASCIISTRING_H
#define UI_DIALOGASCIISTRING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>

QT_BEGIN_NAMESPACE

class Ui_DialogASCIIString
{
public:
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *txtASCII;
    QListWidget *listWidget;
    QCheckBox *chkCaseSensitive;
    QHBoxLayout *hboxLayout;
    QPushButton *btnClose;
    QPushButton *btnHelp;
    QSpacerItem *spacerItem;
    QPushButton *btnFind;
    QProgressBar *progressBar;

    void setupUi(QDialog *DialogASCIIString)
    {
        if (DialogASCIIString->objectName().isEmpty())
            DialogASCIIString->setObjectName(QString::fromUtf8("DialogASCIIString"));
        DialogASCIIString->resize(377, 293);
        gridLayout = new QGridLayout(DialogASCIIString);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(DialogASCIIString);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        txtASCII = new QLineEdit(DialogASCIIString);
        txtASCII->setObjectName(QString::fromUtf8("txtASCII"));

        gridLayout->addWidget(txtASCII, 0, 1, 1, 1);

        listWidget = new QListWidget(DialogASCIIString);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        listWidget->setFont(font);
        listWidget->setAlternatingRowColors(true);

        gridLayout->addWidget(listWidget, 1, 0, 1, 2);

        chkCaseSensitive = new QCheckBox(DialogASCIIString);
        chkCaseSensitive->setObjectName(QString::fromUtf8("chkCaseSensitive"));
        chkCaseSensitive->setEnabled(false);
        chkCaseSensitive->setChecked(true);

        gridLayout->addWidget(chkCaseSensitive, 2, 0, 1, 2);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        btnClose = new QPushButton(DialogASCIIString);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        hboxLayout->addWidget(btnClose);

        btnHelp = new QPushButton(DialogASCIIString);
        btnHelp->setObjectName(QString::fromUtf8("btnHelp"));
        btnHelp->setEnabled(false);

        hboxLayout->addWidget(btnHelp);

        spacerItem = new QSpacerItem(99, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        btnFind = new QPushButton(DialogASCIIString);
        btnFind->setObjectName(QString::fromUtf8("btnFind"));
        btnFind->setDefault(true);

        hboxLayout->addWidget(btnFind);


        gridLayout->addLayout(hboxLayout, 3, 0, 1, 2);

        progressBar = new QProgressBar(DialogASCIIString);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));

        gridLayout->addWidget(progressBar, 4, 0, 1, 2);


        retranslateUi(DialogASCIIString);
        QObject::connect(btnClose, SIGNAL(clicked()), DialogASCIIString, SLOT(close()));

        QMetaObject::connectSlotsByName(DialogASCIIString);
    } // setupUi

    void retranslateUi(QDialog *DialogASCIIString)
    {
        DialogASCIIString->setWindowTitle(QApplication::translate("DialogASCIIString", "Find ASCII String", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DialogASCIIString", "ASCII String", 0, QApplication::UnicodeUTF8));
        chkCaseSensitive->setText(QApplication::translate("DialogASCIIString", "Case Sensitive", 0, QApplication::UnicodeUTF8));
        btnClose->setText(QApplication::translate("DialogASCIIString", "&Close", 0, QApplication::UnicodeUTF8));
        btnHelp->setText(QApplication::translate("DialogASCIIString", "&Help", 0, QApplication::UnicodeUTF8));
        btnFind->setText(QApplication::translate("DialogASCIIString", "&Find", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogASCIIString: public Ui_DialogASCIIString {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGASCIISTRING_H
