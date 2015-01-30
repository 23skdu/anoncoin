// Copyright (c) 2011-2014 The Bitcoin developers
// Copyright (c) 2013-2015 The Anoncoin Core developers
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
    int paddingTop              = 236;
    int paddingCopyrightTop     = 18;

    float fontFactor            = 1.0;

    // define text to place
    QString versionText     = QString("VERSION %1").arg(QString::fromStdString(FormatFullVersion()));
    QString copyrightText1   = QChar(0xA9)+QString(" 2013-%1 ").arg(COPYRIGHT_YEAR) + QString(tr("ANONCOIN CORE DEVELOPERS"));
    //QString testnetAddText  = QString(tr("[testnet]")); // This string is already included in the background image

    QString font            = "Courier New";

    // load the bitmap for writing some text over it
    QPixmap newPixmap;
    if(isTestNet) {
        newPixmap     = QPixmap(":/images/splash_testnet");
    }
    else {
        newPixmap     = QPixmap(":/images/splash");
    }

    QPainter pixPaint(&newPixmap);
    pixPaint.setPen(QColor(250,250,250));
    pixPaint.setFont(QFont(font, 12*fontFactor));
    
    QFontMetrics fm = pixPaint.fontMetrics();

    // draw version
    pixPaint.drawText(newPixmap.width()/2-fm.width(versionText)/2,paddingTop,versionText);

    // draw copyright stuff
    pixPaint.setFont(QFont(font, 12*fontFactor));
    pixPaint.drawText(newPixmap.width()/2-fm.width(copyrightText1)/2,paddingTop+paddingCopyrightTop,copyrightText1);

    // draw testnet string if testnet is on. This is no longer necessary as this is included in the background image
    //if(isTestNet) {
    //    QFont boldFont = QFont(font, 10*fontFactor);
    //    boldFont.setWeight(QFont::Bold);
    //    pixPaint.setFont(boldFont);
    //    fm = pixPaint.fontMetrics();
    //    int testnetAddTextWidth  = fm.width(testnetAddText);
    //    pixPaint.drawText(newPixmap.width()-testnetAddTextWidth-10,15,testnetAddText);
    //}

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
	QFont initfont;
	initfont.setFamily("Courier New");
	initfont.setPixelSize(12);
	initfont.setCapitalization(initfont.AllUppercase);
	splash->setFont(initfont);

	std::string message_cr;
	message_cr = message + "\n";
	
    QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message_cr)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(0,0,0)));
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
