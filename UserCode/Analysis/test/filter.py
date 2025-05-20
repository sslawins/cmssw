#!/cvmfs/cms.cern.ch/el8_amd64_gcc12/cms/cmssw/CMSSW_14_1_1/external/el8_amd64_gcc12/bin/python3

import FWCore.ParameterSet.Config as cms

import os
import sys
import subprocess



from Configuration.Eras.Era_Run3_cff import Run3
process = cms.Process("Filter", Run3)

# MessageLogger & co.
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(1)
process.MessageLogger.suppressWarning  = cms.untracked.vstring('Geometry','AfterSource','L1T')
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))
# process.MessageLogger.cerr.threshold = "DEBUG"
# process.MessageLogger.debugModules = ["*"]

dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToMuMuGamma_MCTunesRun3ECM13p6TeV/BsToMuMuGamma_CMSSW_12_4_11_patch3_14_12_2024/241214_121515/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_Pi0ToGammaGamma_Pi0PythiaGun/Pi0ToGammaGamma_CMSSW_12_4_11_patch3_12_12_2024/241212_131944/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_06_12_2024/241221_163854/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/241223_125459/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToPhiGamma_MCTunesRun3ECM13p6TeV/BsToPhiGamma_CMSSW_12_4_11_patch3_06_12_2024/241206_105826/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BsToKStarGamma_MCTunesRun3ECM13p6TeV/BsToKStarGamma_CMSSW_12_4_11_patch3_02_01_2024/250110_222730/0000/'
# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToKStarGamma_MCTunesRun3ECM13p6TeV/BdToKStarGamma_CMSSW_12_4_11_patch3_23_12_2024/250113_144624/0000/'

# dataDir = '/eos/cms/store/group/phys_bphys/privateMC_ForBsMMGAnalysis/TrackingVertexing/Private_BdToMuMuGamma_MCTunesRun3ECM13p6TeV/BdToMuMuGamma_CMSSW_12_4_11_patch3_15_01_2024/250115_124459/0000/'

lsCommand = 'ls -1 ' + dataDir + '| grep root'
#print('Command: ', lsCommand)

dir = subprocess.Popen(lsCommand, stdout=subprocess.PIPE, shell=True, text=True)
lsOutput = dir.communicate()[0]

files = []
for f in lsOutput.split():
    #print(dataDir + f)
    files.append('file:' + dataDir + f)  # Full path to the files with 'file:' prefix

print('Number of files: ', len(files))

# input files (up to 255 files accepted)

# process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring("file:private_BsToMuMuGamma_Run3_102.root") )
process.source = cms.Source('PoolSource', fileNames =cms.untracked.vstring(files) )
process.source.skipEvents = cms.untracked.uint32(0)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(500000))

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


process.filter= cms.EDFilter("MyFilter")


process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName = cms.untracked.string("BsToMuMuGamma_filtered.root"),
    SelectEvents = cms.untracked.PSet(SelectEvents = cms.vstring('MyPath'))
)
process.MyPath = cms.Path(process.filter)

process.outpath = cms.EndPath(process.out)

# process.schedule = cms.Schedule(process.MyPath, process.outpath)


print("All files set for analysis.")