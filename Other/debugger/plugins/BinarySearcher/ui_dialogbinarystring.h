/********************************************************************************
** Form generated from reading UI file 'dialogbinarystring.ui'
**
** Created: Sat Sep 15 17:40:55 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGBINARYSTRING_H
#define UI_DIALOGBINARYSTRING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListWidget>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include "BinaryString.h"

QT_BEGIN_NAMESPACE

class Ui_DialogBinaryString
{
public:
    QGridLayout *gridLayout;
    BinaryString *binaryString;
    QListWidget *listWidget;
    QCheckBox *chkSkipNoAccess;
    QCheckBox *chkCaseSensitive;
    QCheckBox *chkAlignment;
    QComboBox *cmbAlignment;
    QHBoxLayout *hboxLayout;
    QPushButton *btnClose;
    QPushButton *btnHelp;
    QSpacerItem *spacerItem;
    QPushButton *btnFind;
    QProgressBar *progressBar;

    void setupUi(QDialog *DialogBinaryString)
    {
        if (DialogBinaryString->objectName().isEmpty())
            DialogBinaryString->setObjectName(QString::fromUtf8("DialogBinaryString"));
        DialogBinaryString->resize(447, 376);
        gridLayout = new QGridLayout(DialogBinaryString);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        binaryString = new BinaryString(DialogBinaryString);
        binaryString->setObjectName(QString::fromUtf8("binaryString"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(binaryString->sizePolicy().hasHeightForWidth());
        binaryString->setSizePolicy(sizePolicy);

        gridLayout->addWidget(binaryString, 0, 0, 1, 2);

        listWidget = new QListWidget(DialogBinaryString);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QFont font;
        font.setFamily(QString::fromUtf8("Monospace"));
        listWidget->setFont(font);
        listWidget->setAlternatingRowColors(true);

        gridLayout->addWidget(listWidget, 1, 0, 1, 2);

        chkSkipNoAccess = new QCheckBox(DialogBinaryString);
        chkSkipNoAccess->setObjectName(QString::fromUtf8("chkSkipNoAccess"));

        gridLayout->addWidget(chkSkipNoAccess, 2, 0, 1, 2);

        chkCaseSensitive = new QCheckBox(DialogBinaryString);
        chkCaseSensitive->setObjectName(QString::fromUtf8("chkCaseSensitive"));
        chkCaseSensitive->setEnabled(false);
        chkCaseSensitive->setChecked(true);

        gridLayout->addWidget(chkCaseSensitive, 3, 0, 1, 2);

        chkAlignment = new QCheckBox(DialogBinaryString);
        chkAlignment->setObjectName(QString::fromUtf8("chkAlignment"));

        gridLayout->addWidget(chkAlignment, 4, 0, 1, 1);

        cmbAlignment = new QComboBox(DialogBinaryString);
        cmbAlignment->setObjectName(QString::fromUtf8("cmbAlignment"));

        gridLayout->addWidget(cmbAlignment, 4, 1, 1, 1);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        btnClose = new QPushButton(DialogBinaryString);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));

        hboxLayout->addWidget(btnClose);

        btnHelp = new QPushButton(DialogBinaryString);
        btnHelp->setObjectName(QString::fromUtf8("btnHelp"));
        btnHelp->setEnabled(false);

        hboxLayout->addWidget(btnHelp);

        spacerItem = new QSpacerItem(99, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        btnFind = new QPushButton(DialogBinaryString);
        btnFind->setObjectName(QString::fromUtf8("btnFind"));
        btnFind->setDefault(true);

        hboxLayout->addWidget(btnFind);


        gridLayout->addLayout(hboxLayout, 5, 0, 1, 2);

        progressBar = new QProgressBar(DialogBinaryString);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));

        gridLayout->addWidget(progressBar, 6, 0, 1, 2);

        QWidget::setTabOrder(listWidget, chkSkipNoAccess);
        QWidget::setTabOrder(chkSkipNoAccess, chkCaseSensitive);
        QWidget::setTabOrder(chkCaseSensitive, chkAlignment);
        QWidget::setTabOrder(chkAlignment, cmbAlignment);
        QWidget::setTabOrder(cmbAlignment, btnClose);
        QWidget::setTabOrder(btnClose, btnHelp);
        QWidget::setTabOrder(btnHelp, btnFind);

        retranslateUi(DialogBinaryString);
        QObject::connect(btnClose, SIGNAL(clicked()), DialogBinaryString, SLOT(reject()));

        cmbAlignment->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(DialogBinaryString);
    } // setupUi

    void retranslateUi(QDialog *DialogBinaryString)
    {
        DialogBinaryString->setWindowTitle(QApplication::translate("DialogBinaryString", "Binary String", 0, QApplication::UnicodeUTF8));
        chkSkipNoAccess->setText(QApplication::translate("DialogBinaryString", "Skip Regions With No Access Rights", 0, QApplication::UnicodeUTF8));
        chkCaseSensitive->setText(QApplication::translate("DialogBinaryString", "Case Sensitive", 0, QApplication::UnicodeUTF8));
        chkAlignment->setText(QApplication::translate("DialogBinaryString", "Show Results With This Address Alignment", 0, QApplication::UnicodeUTF8));
        cmbAlignment->clear();
        cmbAlignment->insertItems(0, QStringList()
         << QApplication::translate("DialogBinaryString", "2 Byte Alignment", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("DialogBinaryString", "4 Byte Alignment", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("DialogBinaryString", "8 Byte Alignment", 0, QApplication::UnicodeUTF8)
        );
        btnClose->setText(QApplication::translate("DialogBinaryString", "&Close", 0, QApplication::UnicodeUTF8));
        btnHelp->setText(QApplication::translate("DialogBinaryString", "&Help", 0, QApplication::UnicodeUTF8));
        btnFind->setText(QApplication::translate("DialogBinaryString", "&Find", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogBinaryString: public Ui_DialogBinaryString {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGBINARYSTRING_H
