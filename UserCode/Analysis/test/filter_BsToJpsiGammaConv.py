#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3

import FWCore.ParameterSet.Config as cms
import HeavyFlavorAnalysis.Onia2MuMu.OniaPhotonConversionProducer_cfi

import os
import sys
import subprocess
import glob

process = cms.Process("Filtering")

# MessageLogger & co.
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1)
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))


files = []

for f in glob.glob('/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToJpsiGamma_MCTunesRun3ECM13p6TeV/*/*/*/*.root'):
    files.append('file:' + f)

print('Number of files: ', len(files))

# input files (up to 255 files accepted)

# process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring("file:") )
process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring(files) )
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(2000000))

process.load("TrackingTools/TransientTrack/TransientTrackBuilder_cfi")

process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
# process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_data', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run3_mc_FULL','')

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1)
process.MessageLogger.suppressWarning  = cms.untracked.vstring('Geometry','AfterSource','L1T')
process.options = cms.untracked.PSet( wantSummary=cms.untracked.bool(False))



process.oniaPhotonCandidates = HeavyFlavorAnalysis.Onia2MuMu.OniaPhotonConversionProducer_cfi.PhotonCandidates.clone()
# process.oniaPhotonCandidates.conversions 
process.oniaPhotonCandidates.primaryVertexTag = cms.InputTag('offlinePrimaryVerticesWithBS')



process.filter= cms.EDFilter("FilterConv")


process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string("condor_output/BsToJpsiGamma_conv.root"),
    SelectEvents = cms.untracked.PSet(SelectEvents = cms.vstring('MyPath'))
)
process.MyPath = cms.Path(process.oniaPhotonCandidates*process.filter)

process.outpath = cms.EndPath(process.out)

print("All files set for analysis.")