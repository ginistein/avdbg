/********************************************************************************
** Form generated from reading UI file 'dialog_inputvalue.ui'
**
** Created: Sat Sep 15 17:39:58 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOG_INPUTVALUE_H
#define UI_DIALOG_INPUTVALUE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogInputValue
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *label;
    QLineEdit *hexInput;
    QLabel *label_2;
    QLineEdit *signedInput;
    QLabel *label_3;
    QLineEdit *unsignedInput;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogInputValue)
    {
        if (DialogInputValue->objectName().isEmpty())
            DialogInputValue->setObjectName(QString::fromUtf8("DialogInputValue"));
        DialogInputValue->resize(228, 198);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DialogInputValue->sizePolicy().hasHeightForWidth());
        DialogInputValue->setSizePolicy(sizePolicy);
        DialogInputValue->setModal(true);
        vboxLayout = new QVBoxLayout(DialogInputValue);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
        label = new QLabel(DialogInputValue);
        label->setObjectName(QString::fromUtf8("label"));

        vboxLayout->addWidget(label);

        hexInput = new QLineEdit(DialogInputValue);
        hexInput->setObjectName(QString::fromUtf8("hexInput"));

        vboxLayout->addWidget(hexInput);

        label_2 = new QLabel(DialogInputValue);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        vboxLayout->addWidget(label_2);

        signedInput = new QLineEdit(DialogInputValue);
        signedInput->setObjectName(QString::fromUtf8("signedInput"));

        vboxLayout->addWidget(signedInput);

        label_3 = new QLabel(DialogInputValue);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        vboxLayout->addWidget(label_3);

        unsignedInput = new QLineEdit(DialogInputValue);
        unsignedInput->setObjectName(QString::fromUtf8("unsignedInput"));

        vboxLayout->addWidget(unsignedInput);

        buttonBox = new QDialogButtonBox(DialogInputValue);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);

        vboxLayout->addWidget(buttonBox);

#ifndef QT_NO_SHORTCUT
        label->setBuddy(hexInput);
        label_2->setBuddy(signedInput);
        label_3->setBuddy(unsignedInput);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(hexInput, signedInput);
        QWidget::setTabOrder(signedInput, unsignedInput);

        retranslateUi(DialogInputValue);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogInputValue, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogInputValue, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogInputValue);
    } // setupUi

    void retranslateUi(QDialog *DialogInputValue)
    {
        DialogInputValue->setWindowTitle(QApplication::translate("DialogInputValue", "Input Value", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("DialogInputValue", "Hexdecimal", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("DialogInputValue", "Signed", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("DialogInputValue", "Unsigned", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogInputValue: public Ui_DialogInputValue {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOG_INPUTVALUE_H
