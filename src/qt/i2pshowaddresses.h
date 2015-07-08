// Copyright (c) 2013-2014 The Anoncoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHOWI2PADDRESSES_H
#define SHOWI2PADDRESSES_H

#include <QDialog>

namespace Ui {
class ShowI2PAddresses;
}

class ShowI2PAddresses : public QDialog
{
    Q_OBJECT

public:
    explicit ShowI2PAddresses(QWidget *parent = 0);
    void UpdateParameters( void );
    ~ShowI2PAddresses();

private:
    Ui::ShowI2PAddresses *ui;

private slots:
    void setEnabled( void );
    void setStatic( void );
};

#endif // SHOWI2PADDRESSES_H
