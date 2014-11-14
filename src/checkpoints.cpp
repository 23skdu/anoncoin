// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

#include <stdint.h>

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64_t nTimeLastCheckpoint;
        int64_t nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    bool fEnabled = true;

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 1, uint256("0x8fe2901fc0999bc86ea2668c58802ee87165438166d18154f1bd4f917bf25e0f"))
        ( 7, uint256("0x4530df06d98fc77d04dab427630fc63b45f10d2b0ad3ad3a651883938986d629"))
        ( 7777, uint256("0xae3094030b34a422c44b9832c84fe602d0d528449d6940374bd43b4472b4df5e"))
        (15420, uint256("0xfded6a374d071f59d738a3009fc4d8461609052c3e7e91aa89146550d179c1b0"))
        (16000, uint256("0x683517a8cae8530f39e636f010ecd1750665c3d91f57ba71d6556535972ab328"))
        (77777, uint256("0xf5c98062cb1ad75c792a1851a388447f0edd7cb2271b67ef1241a03c673b7735"))
        (77778, uint256("0xd13f93f9fdac82ea26ed8f90474ed2449c8c24be50a416e43c323a38573c30e5"))
        (100000, uint256("0xcc4f0b11e9e17f7a406ac4a71e6e192b9b43e32b300ddecba229c789392497eb"))
        (125000, uint256("0x52a933553dda61b4af6a418f10ce563f0c84df93619ec97f8f360f9ecc51cf5b"))
        (150000, uint256("0xb1dc241da3e8e7d4ca2df75187d66f4f096840634667b987a7b60d5dde92853c"))
        (210000, uint256("0xb9c2c5030199a87cb99255551b7f67bff6adf3ef2caf97258b93b417d14f9051"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1376885874, // * UNIX timestamp of last checkpoint block
        1859062,   // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        7000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 546, uint256("0xa0fea99a6897f531600c8ae53367b126824fd6a847b2b2b73817a95b8e27e602"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1365458829,
        547,
        576
    };

    static MapCheckpoints mapCheckpointsRegtest =
        boost::assign::map_list_of
        ( 0, uint256("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"))
        ;
    static const CCheckpointData dataRegtest = {
        &mapCheckpointsRegtest,
        0,
        0,
        0
    };

    const CCheckpointData &Checkpoints() {
        if (Params().NetworkID() == CChainParams::TESTNET)
            return dataTestnet;
        else if (Params().NetworkID() == CChainParams::MAIN)
            return data;
        else
            return dataRegtest;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!fEnabled)
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
        if (pindex==NULL)
            return 0.0;

        int64_t nNow = time(NULL);

        double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkpoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!fEnabled)
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!fEnabled)
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
