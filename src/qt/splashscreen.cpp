// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2013-2014 The Anoncoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// Many builder specific things set in the config file, ENABLE_WALLET is a good example.  Don't forget to include it this way in your source files.
#if defined(HAVE_CONFIG_H)
#include "config/anoncoin-config.h"
#endif

#include "splashscreen.h"

#include "clientversion.h"
#include "init.h"
#include "ui_interface.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif

#include <QApplication>
#include <QPainter>

SplashScreen::SplashScreen(const QPixmap &pixmap, Qt::WindowFlags f, bool isTestNet) :
    QSplashScreen(pixmap, f)
{
    setAutoFillBackground(true);

    // set reference point, paddings
    int paddingRight            = 190;
    int paddingRightCopyright   = 220;
    int paddingTop              = 170;
    int paddingCopyrightTop     = 70;
    int titleCopyrightVSpace    = 14;

    float fontFactor            = 1.0;

    // define text to place
    QString versionText     = QString("Version %1").arg(QString::fromStdString(FormatFullVersion()));
    QString copyrightText1   = QChar(0xA9)+QString(" 2013-%1 ").arg(COPYRIGHT_YEAR) + QString(tr("The Anoncoin Core developers"));
    QString copyrightText2   = QChar(0xA9)+QString(" 2009-%1 ").arg(2014) + QString(tr("The Bitcoin Core developers"));
    QString copyrightText3   = QChar(0xA9)+QString(" 2011-%1 ").arg(2014) + QString(tr("The Litecoin Core developers"));
    QString copyrightText4   = QChar(0xA9)+QString(" 2013 ") + QString(tr("The Primecoin developers"));
    QString testnetAddText  = QString(tr("[testnet]")); // define text to place as single text object

    QString font            = "Arial";

    // load the bitmap for writing some text over it
    QPixmap newPixmap;
    if(isTestNet) {
        newPixmap     = QPixmap(":/images/splash_testnet");
    }
    else {
        newPixmap     = QPixmap(":/images/splash");
    }

    QPainter pixPaint(&newPixmap);
    pixPaint.setPen(QColor(100,100,100));
    pixPaint.setFont(QFont(font, 8*fontFactor));

    QFontMetrics fm = pixPaint.fontMetrics();

    // draw version
    pixPaint.drawText(newPixmap.width()-paddingRight+2,paddingTop,versionText);

    // draw copyright stuff
    pixPaint.setFont(QFont(font, 8*fontFactor));
    pixPaint.drawText(newPixmap.width()-paddingRightCopyright,paddingTop+paddingCopyrightTop,copyrightText1);
    pixPaint.drawText(newPixmap.width()-paddingRightCopyright,paddingTop+paddingCopyrightTop+titleCopyrightVSpace,copyrightText2);
    pixPaint.drawText(newPixmap.width()-paddingRightCopyright,paddingTop+paddingCopyrightTop+titleCopyrightVSpace*2,copyrightText3);
    pixPaint.drawText(newPixmap.width()-paddingRightCopyright,paddingTop+paddingCopyrightTop+titleCopyrightVSpace*3,copyrightText4);

    // draw testnet string if testnet is on
    if(isTestNet) {
        QFont boldFont = QFont(font, 10*fontFactor);
        boldFont.setWeight(QFont::Bold);
        pixPaint.setFont(boldFont);
        fm = pixPaint.fontMetrics();
        int testnetAddTextWidth  = fm.width(testnetAddText);
        pixPaint.drawText(newPixmap.width()-testnetAddTextWidth-10,15,testnetAddText);
    }

    pixPaint.end();

    this->setPixmap(newPixmap);

    subscribeToCoreSignals();
}

SplashScreen::~SplashScreen()
{
    unsubscribeFromCoreSignals();
}

void SplashScreen::slotFinish(QWidget *mainWin)
{
    finish(mainWin);
}

static void InitMessage(SplashScreen *splash, const std::string &message)
{
    QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(55,55,55)));

}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress)
{
    InitMessage(splash, title + strprintf("%d", nProgress) + "%");
}

#ifdef ENABLE_WALLET
static void ConnectWallet(SplashScreen *splash, CWallet* wallet)
{
    wallet->ShowProgress.connect(boost::bind(ShowProgress, splash, _1, _2));
}
#endif

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.InitMessage.connect(boost::bind(InitMessage, this, _1));
#ifdef ENABLE_WALLET
    uiInterface.LoadWallet.connect(boost::bind(ConnectWallet, this, _1));
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.InitMessage.disconnect(boost::bind(InitMessage, this, _1));
#ifdef ENABLE_WALLET
    if(pwalletMain)
        pwalletMain->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
#endif
}
