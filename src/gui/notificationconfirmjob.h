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

#ifndef NOTIFICATIONCONFIRMJOB_H
#define NOTIFICATIONCONFIRMJOB_H

#include "accountfwd.h"
#include "abstractnetworkjob.h"

#include <QVector>
#include <QList>
#include <QPair>
#include <QUrl>

namespace OCC {

class NotificationWidget;

/**
 * @brief The NotificationConfirmJob class
 * @ingroup gui
 *
 * Class to call an action-link of a notification coming from the server.
 * All the communication logic is handled in this class.
 *
 */
class NotificationConfirmJob : public AbstractNetworkJob {
    Q_OBJECT

public:

    explicit NotificationConfirmJob(AccountPtr account);

    /**
     * Set the verb and link for the job
     *
     * @param verb currently supported GET PUT POST DELETE
     */
    void setLinkAndVerb(const QUrl& link, const QString &verb);

    /**
     * Start the OCS request
     */
    void start() Q_DECL_OVERRIDE;

    void setWidget( NotificationWidget *widget );

    NotificationWidget *widget();

signals:

    /**
     * Result of the OCS request
     *
     * @param reply the reply
     */
    void jobFinished(QString reply, int replyCode);

private slots:
    virtual bool finished() Q_DECL_OVERRIDE;

private:
    QString _verb;
    QUrl _link;
    NotificationWidget *_widget;
};

}

#endif // NotificationConfirmJob_H