/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "notificationwidget.h"
#include "QProgressIndicator.h"

#include <QPushButton>

namespace OCC {

NotificationWidget::NotificationWidget(QWidget *parent) : QWidget(parent)
{
    _ui.setupUi(this);
    _progressIndi = new QProgressIndicator(this);
    _ui.horizontalLayout->addWidget(_progressIndi);
}

void NotificationWidget::setAccountName( const QString& name )
{
    _accountName = name;
}

void NotificationWidget::setActivity(const Activity& activity)
{
    _myActivity = activity;

    Q_ASSERT( !activity._accName.isEmpty() );
    _accountName = activity._accName;

    // _ui._headerLabel->setText( );
    _ui._subjectLabel->setText(activity._subject);
    if( activity._message.isEmpty()) {
        _ui._messageLabel->hide();
    } else {
        _ui._messageLabel->setText(activity._message);
    }
    _ui._notifIcon->setPixmap(QPixmap(":/client/resources/bell.png"));
    _ui._notifIcon->setMinimumWidth(64);
    _ui._notifIcon->setMinimumHeight(64);
    _ui._notifIcon->show();

    // always remove the buttons
    foreach( auto button, _ui._buttonBox->buttons() ) {
        _ui._buttonBox->removeButton(button);
    }

    // display buttons for the links
    foreach( auto link, activity._links ) {
        QPushButton *b = _ui._buttonBox->addButton( link._label, QDialogButtonBox::AcceptRole);
        b->setDefault(link._isPrimary);
        connect(b, SIGNAL(clicked()), this, SLOT(slotButtonClicked()));
        _buttons.append(b);
    }
}

void NotificationWidget::slotButtonClicked()
{
    QObject *buttonWidget = QObject::sender();
    int index = -1;
    if( buttonWidget ) {
        // find the button that was clicked, it has to be in the list
        // of buttons that were added to the button box before.
        for( int i = 0; i < _buttons.count(); i++ ) {
            if( _buttons.at(i) == buttonWidget ) {
                index = i;
            }
            _buttons.at(i)->setEnabled(false);
        }

        // if the button was found, the link must be called
        if( index > -1 && index < _myActivity._links.count() ) {
            ActivityLink triggeredLink = _myActivity._links.at(index);
            qDebug() << "Notification Link: "<< triggeredLink._verb << triggeredLink._link;
            _progressIndi->startAnimation();
            emit sendNotificationRequest( _accountName, triggeredLink._link, triggeredLink._verb );
        }
    }
}

void NotificationWidget::slotNotificationRequestFinished(int statusCode)
{
    int i = 0;
    // the ocs API returns stat code 100 if it succeeded.
    if( statusCode != 100 ) {
        qDebug() << "Notification Request to Server failed, leave button visible.";
        for( i = 0; i < _buttons.count(); i++ ) {
            _buttons.at(i)->setEnabled(true);
        }
    } else {
        // the call to the ocs API succeeded.
        _ui._buttonBox->hide();

    }
    _progressIndi->stopAnimation();
}

}