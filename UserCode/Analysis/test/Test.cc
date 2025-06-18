#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "RecoVertex/KinematicFitPrimitives/interface/Matrices.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "RecoVertex/KinematicFitPrimitives/interface/ParticleMass.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicTree.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/PointingKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/SimplePointingConstraint.h"
#include "RecoVertex/KinematicFit/interface/SmartPointingConstraint.h"
#include "RecoVertex/KinematicFit/interface/MultiTrackPointingKinematicConstraint.h"


#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/TrajectoryParametrization/interface/CartesianTrajectoryError.h"
#include "DataFormats/TrajectoryState/interface/TrackCharge.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/Math/interface/AlgebraicROOTObjects.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

#include "DataFormats/GeometrySurface/interface/ReferenceCounted.h"



#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TMatrixD.h"
#include "Math/ProbFunc.h"

#include <sstream>
#include <iomanip> 
#include <utility>
#include <numeric>
#include <vector>


using namespace std;


//object definition
class Test : public edm::one::EDAnalyzer<> {
public:

  //constructor, function is called when new object is created
  explicit Test(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~Test();

  //edm filter plugin specific functions
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob();

  bool isSameDecay(const std::vector<int>&, const std::vector<int>&);
private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;
  edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> m_fieldToken;
  edm::EDGetTokenT < edm::TriggerResults > theTriggerResultsToken;
  edm::ESGetToken <TransientTrackBuilder, TransientTrackRecord> theTransientTrackBuilderToken;

  edm::EDGetTokenT<EcalRecHitCollection> ebRecHitsToken_;
  edm::EDGetTokenT<EcalRecHitCollection> eeRecHitsToken_;

  edm::EDGetTokenT < reco::BeamSpot > theBeamSpotToken;
  edm::EDGetTokenT < vector<reco::Vertex> > theVertexToken;

  unique_ptr<EcalClusterLazyToolsBase::ESGetTokens> esGetTokens;

  // histograms

  TH1D* hBsMass;
  TH1D* hBsMassGlobal;
  TH1D* hBsMassPointing;
  TH1D* hBsMassFromP4;
  TH1D* hMuPt;
  TH1D* hGammaPt;
  TH1D* hGammaPtReco;
  TH1D* hGammaDeltaR;
  TH1D* hGammaPtWithTrigger;
  
  TH1D* hDistance;
  TH1D* hDistanceGlobal;
  TH1D* hDistancePointing;
  TH1D* hPhotonCosineSimilarity;
  TH1D* h2vs3Distance;
  TH1D* hNormalVsPointingDistance;
  TH1D* hDistanceFromPV;
  TH1D* hDistanceFromPVGlobal;

  TH1D* hCosSimBsVsSV;

  TH1D* hMomSVPVAngle;
  TH1D* hMomSVPVAngleGen;

  TH1D* hMuonPtResReco;
  TH1D* hMuonPtResFit;
  TH1D* hPhotonPtResReco;
  TH1D* hPhotonPtResFit;

  TH1D* hPhotonEnergyResReco;
  TH1D* hPhotonEnergyResFit;
  TH1D* hPhotonEnergyRecoVsFit;

  TH1D* hPVRecoVsGenDistance;
  TH1D* hPVSVDistanceGen;

  TH1D* hBsMassResidual;
  TH1D* hXResidual;
  TH1D* hZResidual;
  TH1D* hChisquaredProb;

  TH1D* hBsMassResidualGlobal;
  TH1D* hXResidualGlobal;
  TH1D* hZResidualGlobal;
  TH1D* hChisquaredProbGlobal;

  TH1D* hEnergyPull;
  TH1D* hEtaPull;
  TH1D* hPhiPull;

  TH1D* hEnergyResidual;
  TH1D* hEtaResidual;
  TH1D* hPhiResidual;

  TH1D* hXPull;
  TH1D* hYPull;
  TH1D* hZPull;

  TH1D* hCaloXResidual;
  TH1D* hCaloYResidual;
  TH1D* hCaloZResidual;

  TH1D* hCaloR;

  TH1D* hDimuonVertexVsMMGVertexDistance;
  TH1D* hDimuonDistance;

  int nConvPhotons = 0;
  int nGenMatchedEvents = 0;
  std::vector<int> MuMuG = {22, 13, -13};
};


Test::Test(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theGenParticleToken = consumes< vector<reco::GenParticle>  >( edm::InputTag("genParticles"));
  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
  m_fieldToken = esConsumes<MagneticField, IdealMagneticFieldRecord>();
  theTriggerResultsToken = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
  theTransientTrackBuilderToken = esConsumes(edm::ESInputTag("", "TransientTrackBuilder"));
  ebRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));
  eeRecHitsToken_ = consumes<EcalRecHitCollection>(edm::InputTag("reducedEcalRecHitsEB"));

  esGetTokens = make_unique<EcalClusterLazyToolsBase::ESGetTokens>(consumesCollector());

  theBeamSpotToken = consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot"));
  theVertexToken = consumes< vector<reco::Vertex>  >( edm::InputTag("offlinePrimaryVertices"));

}

Test::~Test()
{
  cout <<" DTOR" << endl;
}

bool Test::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void Test::beginJob()
{
  //create a histogram

  hBsMass = new TH1D("hBsMass", "hBsMass", 50, 3, 7);
  hBsMassGlobal = new TH1D("hBsMassGlobal", "hBsMassGlobal", 50, 3, 7);
  hBsMassPointing = new TH1D("hBsMassPointing", "hBsMassPointing", 50, 3, 7);
  hBsMassFromP4 = new TH1D("hBsMassFromP4", "hBsMassFromP4", 50, 3, 7);
  hMuPt = new TH1D("hMuPt", "hMuPt", 100, 0, 30);
  hGammaPt = new TH1D("hGammaPt", "hGammaPt", 100, 0, 30);
  hGammaPtReco = new TH1D("hGammaPtReco", "hGammaPtReco", 100, 0, 30);
  hGammaDeltaR = new TH1D("hGammaDeltaR", "hGammaDeltaR", 100, 0, 0.05);
  hGammaPtWithTrigger = new TH1D("hGammaPtWithTrigger", "hGammaPtWithTrigger", 100, 0, 30);

  hDistance = new TH1D("hDistance", "hDistance", 100, 0, 0.1);
  hDistanceGlobal = new TH1D("hDistanceGlobal", "hDistanceGlobal", 100, 0, 0.1);
  hDistancePointing = new TH1D("hDistancePointing", "hDistancePointing", 100, 0, 0.1);
  hPhotonCosineSimilarity = new TH1D("hPhotonCosineSimilarity", "hPhotonCosineSimilarity", 100, -1, 1);
  h2vs3Distance = new TH1D("h2vs3Distance", "h2vs3Distance", 100, 0, 0.1);
  hNormalVsPointingDistance = new TH1D("hNormalVsPointingDistance", "hNormalVsPointingDistance", 100, 0, 0.02);
  hDistanceFromPV = new TH1D("hDistanceFromPV", "hDistanceFromPV", 100, 0, 0.5);
  hDistanceFromPVGlobal = new TH1D("hDistanceFromPVGlobal", "hDistanceFromPVGlobal", 100, 0, 0.5);

  hCosSimBsVsSV = new TH1D("hCosSimBsVsSV", "hCosSimBsVsSV", 100, 0, 30);

  hMomSVPVAngle = new TH1D("hMomSVPVAngle", "hMomSVPVAngle", 100, 0, 30);
  hMomSVPVAngleGen = new TH1D("hMomSVPVAngleGen", "hMomSVPVAngleGen", 100, 0, 30);

  hMuonPtResReco = new TH1D("hMuonPtResReco", "hMuonPtResReco", 100, -0.1, 0.1);
  hMuonPtResFit = new TH1D("hMuonPtResFit", "hMuonPtResFit", 100, -0.1, 0.1);
  hPhotonPtResReco = new TH1D("hPhotonPtResReco", "hPhotonPtResReco", 100, -0.5, 0.5);
  hPhotonPtResFit = new TH1D("hPhotonPtResFit", "hPhotonPtResFit", 100, -0.5, 0.5);

  hPhotonEnergyResReco = new TH1D("hPhotonEnergyResReco", "hPhotonEnergyResReco", 100, -0.5, 0.5);
  hPhotonEnergyResFit = new TH1D("hPhotonEnergyResFit", "hPhotonEnergyResFit", 100, -0.5, 0.5);
  hPhotonEnergyRecoVsFit = new TH1D("hPhotonEnergyRecoVsFit", "hPhotonEnergyRecoVsFit", 100, -0.5, 0.5);

  hPVRecoVsGenDistance = new TH1D("hPVRecoVsGenDistance", "hPVRecoVsGenDistance", 100, 0, 0.1);
  hPVSVDistanceGen = new TH1D("hPVSVDistanceGen", "hPVSVDistanceGen", 100, 0, 0.5);

  hBsMassResidual = new TH1D("hBsMassResidual", "hBsMassResidual", 100, -0.5, 0.5);
  hXResidual = new TH1D("hXResidual", "hXResidual", 100, -0.05, 0.05);
  hZResidual = new TH1D("hZResidual", "hZResidual", 100, -0.05, 0.05);
  hChisquaredProb = new TH1D("hChisquaredProb", "hChisquaredProb", 100, 0, 1);

  hBsMassResidualGlobal = new TH1D("hBsMassResidualGlobal", "hBsMassResidualGlobal", 100, -0.5, 0.5);
  hXResidualGlobal = new TH1D("hXResidualGlobal", "hXResidualGlobal", 100, -0.05, 0.05);
  hZResidualGlobal = new TH1D("hZResidualGlobal", "hZResidualGlobal", 100, -0.05, 0.05);
  hChisquaredProbGlobal = new TH1D("hChisquaredProbGlobal", "hChisquaredProbGlobal", 100, 0, 1);

  hEnergyPull = new TH1D("hEnergyPull", "hEnergyPull", 100, -5, 5);
  hEtaPull = new TH1D("hEtaPull", "hEtaPull", 100, -2, 2);
  hPhiPull = new TH1D("hPhiPull", "hPhiPull", 100, -2, 2);

  hEnergyResidual = new TH1D("hEnergyResidual", "hEnergyResidual", 100, -2, 2);
  hEtaResidual = new TH1D("hEtaResidual", "hEtaResidual", 100, -2, 2);
  hPhiResidual = new TH1D("hPhiResidual", "hPhiResidual", 100, -3.14, 3.14);

  hXPull = new TH1D("hXPull", "hXPull", 100, -2, 2);
  hYPull = new TH1D("hYPull", "hYPull", 100, -2, 2);
  hZPull = new TH1D("hZPull", "hZPull", 100, -2, 2);

  hCaloXResidual = new TH1D("hCaloXResidual", "hCaloXResidual; [cm]", 100, -5, 5);
  hCaloYResidual = new TH1D("hCaloYResidual", "hCaloYResidual; [cm]", 100, -5, 5);
  hCaloZResidual = new TH1D("hCaloZResidual", "hCaloZResidual; [cm]", 100, -5, 5);


  hCaloR = new TH1D("hCaloR", "hCaloR", 100, 0, 200);

  hDimuonVertexVsMMGVertexDistance = new TH1D("hDimuonVertexVsMMGVertexDistance", "hDimuonVertexVsMMGVertexDistance; [cm]", 100, 0, 0.1);
  hDimuonDistance = new TH1D("hDimuonDistance", "hDimuonDistance; [cm]", 100, 0, 0.1);

  cout << "HERE Test::beginJob()" << endl;
}

void Test::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data

  hBsMass->Write();
  hBsMassGlobal->Write();
  hBsMassPointing->Write();
  hBsMassFromP4->Write();
  hMuPt->Write();
  hGammaPt->Write();
  hGammaPtReco->Write();
  hGammaDeltaR->Write();
  hGammaPtWithTrigger->Write();

  hDistance->Write();
  hDistanceGlobal->Write();
  hDistancePointing->Write();
  hPhotonCosineSimilarity->Write();
  h2vs3Distance->Write();
  hNormalVsPointingDistance->Write();
  hDistanceFromPV->Write();
  hDistanceFromPVGlobal->Write();

  hCosSimBsVsSV->Write();

  hMomSVPVAngle->Write();
  hMomSVPVAngleGen->Write();

  hMuonPtResReco->Write();
  hMuonPtResFit->Write();
  hPhotonPtResReco->Write();
  hPhotonPtResFit->Write();

  hPhotonEnergyResReco->Write();
  hPhotonEnergyResFit->Write();
  hPhotonEnergyRecoVsFit->Write();

  hPVRecoVsGenDistance->Write();
  hPVSVDistanceGen->Write();

  hBsMassResidual->Write();
  hXResidual->Write();
  hZResidual->Write();
  hChisquaredProb->Write();

  hBsMassResidualGlobal->Write();
  hXResidualGlobal->Write();
  hZResidualGlobal->Write();
  hChisquaredProbGlobal->Write();

  hEnergyPull->Write();
  hEtaPull->Write();
  hPhiPull->Write();

  hEnergyResidual->Write();
  hEtaResidual->Write();
  hPhiResidual->Write();

  hXPull->Write();
  hYPull->Write();
  hZPull->Write();

  hCaloXResidual->Write();
  hCaloYResidual->Write();
  hCaloZResidual->Write();


  hCaloR->Write();

  hDimuonVertexVsMMGVertexDistance->Write();
  hDimuonDistance->Write();
  
  myRootFile.Close();

  delete hBsMass;
  delete hBsMassGlobal;
  delete hBsMassPointing;
  delete hBsMassFromP4;
  delete hMuPt;
  delete hGammaPt;
  delete hGammaPtReco;
  delete hGammaDeltaR;
  delete hGammaPtWithTrigger;

  delete hDistance;
  delete hDistanceGlobal;
  delete hDistancePointing;
  delete hPhotonCosineSimilarity;
  delete h2vs3Distance;
  delete hNormalVsPointingDistance;
  delete hDistanceFromPV;
  delete hDistanceFromPVGlobal;

  delete hCosSimBsVsSV;

  delete hMomSVPVAngle;
  delete hMomSVPVAngleGen;

  delete hMuonPtResReco;
  delete hMuonPtResFit;
  delete hPhotonPtResReco;
  delete hPhotonPtResFit;

  delete hPhotonEnergyResReco;
  delete hPhotonEnergyResFit;
  delete hPhotonEnergyRecoVsFit;

  delete hPVRecoVsGenDistance;
  delete hPVSVDistanceGen;

  delete hBsMassResidual;
  delete hXResidual;
  delete hZResidual;
  delete hChisquaredProb;

  delete hBsMassResidualGlobal;
  delete hXResidualGlobal;
  delete hZResidualGlobal;
  delete hChisquaredProbGlobal;

  delete hEnergyPull;
  delete hEtaPull;
  delete hPhiPull;

  delete hEnergyResidual;
  delete hEtaResidual;
  delete hPhiResidual;

  delete hXPull;
  delete hYPull;
  delete hZPull;

  delete hCaloXResidual;
  delete hCaloYResidual;
  delete hCaloZResidual;


  delete hCaloR;

  delete hDimuonVertexVsMMGVertexDistance;
  delete hDimuonDistance;

  cout << "genMatchedEvents: " << nGenMatchedEvents << endl;

  cout << "HERE Test::endJob()" << endl;
}


void Test::analyze(
    const edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE Test::analyze "<< std::endl;

  const std::vector<reco::GenParticle> & genPar = ev.get(theGenParticleToken);
  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);
  auto const& field = es.getData(m_fieldToken);

  const TransientTrackBuilder* theB = &es.getData(theTransientTrackBuilderToken);

  const edm::TriggerResults & triggerResults = ev.get(theTriggerResultsToken);
  edm::TriggerNames triggerNames = ev.triggerNames(triggerResults);

// check trigger
// bool triggerFired = false;
//   for (unsigned int i = 0; i < triggerResults.size(); i++)
//   {
//     TString name = triggerNames.triggerName(i);
//     if(name == "HLT_DoubleMu4_3_LowMass_v1" && triggerResults.accept(i) == 0)
//     {
//       return;
//     }
//   }


  vector<const reco::Candidate*> genMuons;
  vector<const reco::Muon*> recoMatchedMuons;
  vector<const reco::Candidate*> genMatchedMuons;

  vector<const reco::Candidate*> genPhotons;
  vector<const reco::Photon*> recoMatchedPhotons;
  vector<const reco::Candidate*> genMatchedPhotons;

  const reco::BeamSpot & beamSpot = ev.get(theBeamSpotToken);
  const std::vector<reco::Vertex> & primaryVertices = ev.get(theVertexToken);


  // for(const auto& photon: recoPhotons)
  // {
  //   cout << "is EB: " << photon.isEB() << endl;
  //   cout << "is EE: " << photon.isEE() << endl;
  //   EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);
  //   TMatrixD cov = lazyTools.covariancesXYZ(*photon.superCluster());
  //   cov.Print();
  // }

  GlobalPoint genPV;
  GlobalPoint genSV;
  GlobalVector genBsMomentum;

  for(const auto& genP : genPar)
  {
    if (abs(genP.pdgId()) == 531)
    {
      vector<int> daughters;
      for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
      {
        daughters.push_back(genP.daughter(i)->pdgId());
      }
      if(isSameDecay(daughters, MuMuG))
      {
        for(unsigned int i=0; i < genP.numberOfDaughters(); i++)
        {
          if(abs(genP.daughter(i)->pdgId()) == 13) genMuons.push_back(genP.daughter(i));
          if(abs(genP.daughter(i)->pdgId()) == 22) genPhotons.push_back(genP.daughter(i));
        }
        genSV = GlobalPoint(genP.daughter(0)->vx(), genP.daughter(0)->vy(), genP.daughter(0)->vz());
        genBsMomentum = GlobalVector(genP.px(), genP.py(), genP.pz());

        // find the primary vertex
        const reco::Candidate* mother = genP.mother();
        while(mother->mother()->pdgId() != 2212)
        {
          mother = mother->mother();
        }
        genPV = GlobalPoint(mother->vx(), mother->vy(), mother->vz());

      }
    }
  }



  reco::Candidate::Point genPVPoint(genPV.x(), genPV.y(), genPV.z());
  reco::Candidate::Point recoPVPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
  hPVRecoVsGenDistance->Fill((recoPVPoint - genPVPoint).R());

  GlobalVector genPVToSV = genSV - genPV;
  hMomSVPVAngleGen->Fill(acos(genBsMomentum.dot(genPVToSV) / (genBsMomentum.mag() * genPVToSV.mag()))*180./3.14159);

  hPVSVDistanceGen->Fill((genPV - genSV).mag());


  // reco muon matching
  for (const reco::Candidate* genMu : genMuons)
  {
    float minDR = 10;
    const reco::Muon* bestMatchedMuon;
    bool matched = false;
    for (const auto& recoMu : recoMuons)
    {
      float dR = reco::deltaR(recoMu, *genMu);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedMuon = &recoMu;
        matched = true;
      }
    }
    // if (matched) hMuDeltaR->Fill(minDR);
    if (matched && minDR < 0.01)
    {
      recoMatchedMuons.push_back(bestMatchedMuon);
      genMatchedMuons.push_back(genMu);
      // hRecoVsGenMuPt->Fill(genMu->pt(), bestMatchedMuon->pt());
      // hMuPtError->Fill((bestMatchedMuon->pt() - genMu->pt())/genMu->pt());
    }
  }

  // reco photon matching
  for (const reco::Candidate* genPh : genPhotons)
  {
    float minDR = 10;
    const reco::Photon* bestMatchedPhoton;
    bool matched = false;
    for (const auto& recoPh : recoPhotons)
    {
      float dR = reco::deltaR(recoPh, *genPh);
      if (dR < minDR)
      {
        minDR = dR;
        bestMatchedPhoton = &recoPh;
        matched = true;
      }
    }
    // if (matched) hGammaDeltaR->Fill(minDR);
    if (matched && minDR < 0.02)
    {
      reco::Photon* correctedPhoton = new reco::Photon(*bestMatchedPhoton);
      reco::Candidate::Point genPhotonVertex = genPh->vertex();
      reco::Candidate::Vector genPhotonMomentum = genPh->momentum();
      reco::Candidate::Point fakeCaloPosition = genPhotonVertex + genPhotonMomentum.unit() * 100;
      correctedPhoton->setCaloPosition(math::XYZPointF(fakeCaloPosition.x(), fakeCaloPosition.y(), fakeCaloPosition.z()));
      correctedPhoton->setP4(genPh->p4());

      double photonEnergy = bestMatchedPhoton->energy();
      double correctedEnergy = (photonEnergy - 0.599366) / 1.02408;
      double scalingFactor = correctedEnergy / photonEnergy;
      reco::Photon* energyCorrectedPhoton = new reco::Photon(*bestMatchedPhoton);
      energyCorrectedPhoton->setP4(energyCorrectedPhoton->p4() * scalingFactor);

      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
      // hRecoVsGenGammaPt->Fill(genPh->pt(), bestMatchedPhoton->pt());
      // hGammaPtError->Fill((bestMatchedPhoton->pt() - genPh->pt())/genPh->pt());
    }
  }

  if(genMatchedMuons.size() == 2 && genMatchedPhotons.size() == 1)
  {
    if(recoMatchedPhotons[0]->isEB()) nGenMatchedEvents++;
  }

  if(recoMatchedPhotons.size() >0)
  {
    hGammaPt->Fill(genMatchedPhotons[0]->pt());
    hGammaPtReco->Fill(recoMatchedPhotons[0]->pt());
  }




  // kinematic particle creation
  
  vector<RefCountedKinematicParticle> muonKinematicParticles;
  for(const auto& recoMuPtr : recoMatchedMuons)
  {
    reco::Muon recoMu = *recoMuPtr;
    hMuPt->Fill(recoMu.pt());
    reco::TrackRef muTrack = recoMu.track();
    if(!muTrack) continue;
    reco::TransientTrack muonTT = reco::TransientTrack(muTrack, &field);

    const ParticleMass muon_mass(0.105658);
    float muon_sigma = 1E-6;

    KinematicParticleFactoryFromTransientTrack pFactory;
    muonKinematicParticles.push_back(pFactory.particle(muonTT, muon_mass, float(0), float(0), muon_sigma));
  }

  vector<RefCountedKinematicParticle> photonKinematicParticles;
  for(const auto& recoPhoPtr : recoMatchedPhotons)
  {
    reco::Photon recoPho = *recoPhoPtr;
    if(recoPho.isEB() == 0) continue;

    // cout << "photon calo position:" << recoPho.caloPosition() << endl;
    // cout << "photon energy:" << recoPho.energy() << endl;

    GlobalPoint vtx(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
    GlobalVector p3(recoPho.px(), recoPho.py(), recoPho.pz());
    TrackCharge ch = 0;

    const ParticleMass photon_mass(0.);
    float photon_sigma = 1E-6;

    FreeTrajectoryState fts(vtx, p3, ch, &field);


    EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);
    TMatrixD cov(lazyTools.covariancesXYZ(*recoPho.superCluster()));
    
    // for(int i = 0; i < cov.GetNrows(); i++)
    // {
    //   for(int j = 0; j < cov.GetNcols(); j++)
    //   {
    //     if (i == j) cov(i, j) *= 1e-6; // scale the diagonal elements
    //     else cov(i, j) = 0; // off-diagonal elements are set to zero
    //   }
    // }
    TMatrixD* covPtr(new TMatrixD(cov));

    // fill the pull histograms
    hEnergyPull->Fill((recoPho.energy() - genMatchedPhotons[0]->energy()) / recoPho.getCorrectedEnergyError(reco::Photon::ecal_photons));
    hEnergyResidual->Fill((recoPho.energy() - genMatchedPhotons[0]->energy()));
    std::array<float, 3> covEtaPhi = lazyTools.covariances(*recoPho.superCluster());

    reco::Candidate::Point genPhotonVertex = genMatchedPhotons[0]->vertex();
    reco::Candidate::Vector genPhotonMomentum = genMatchedPhotons[0]->momentum();
    reco::Candidate::Point genCaloPosition;

    for(double l = 130; l < 500; l+=0.1)
    {
      genCaloPosition = genPhotonVertex + genPhotonMomentum.unit() * l;
      if (genCaloPosition.rho() >= 136.5) break;
    }

    hEtaPull->Fill((recoPho.caloPosition().eta() - genCaloPosition.eta()) / sqrt(covEtaPhi[0]));
    hPhiPull->Fill(reco::deltaPhi(recoPho.caloPosition().phi(), genCaloPosition.phi()) / sqrt(covEtaPhi[2]));

    hEtaResidual->Fill(recoPho.caloPosition().eta() - genCaloPosition.eta());
    hPhiResidual->Fill(reco::deltaPhi(recoPho.caloPosition().phi(), genCaloPosition.phi()));

    hXPull->Fill((recoPho.caloPosition().x() - genCaloPosition.x()) / sqrt(cov(0, 0)));
    hYPull->Fill((recoPho.caloPosition().y() - genCaloPosition.y()) / sqrt(cov(1, 1)));
    hZPull->Fill((recoPho.caloPosition().z() - genCaloPosition.z()) / sqrt(cov(2, 2)));

    hCaloXResidual->Fill(recoPho.caloPosition().x() - genCaloPosition.x());
    hCaloYResidual->Fill(recoPho.caloPosition().y() - genCaloPosition.y());
    hCaloZResidual->Fill(recoPho.caloPosition().z() - genCaloPosition.z());

    hCaloR->Fill(recoPho.caloPosition().rho());
    
    AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
    AlgebraicVector6 diagonal(1e6, 1e6, 1e6, 1e6, 1e6, 1e6);
    photonCov.SetDiagonal(diagonal);

    CartesianTrajectoryError photonErr(photonCov);
    fts.setCartesianError(photonErr);

    reco::TransientTrack phoTT = theB->build(fts);
    KinematicParticleFactoryFromTransientTrack pFactory;
    photonKinematicParticles.push_back(pFactory.particle(phoTT, photon_mass, float(0), float(0), photon_sigma, recoPhoPtr, covPtr));

  }

  for (unsigned int i = 0; i < muonKinematicParticles.size(); i++)
  {
    for (unsigned int j = i+1; j < muonKinematicParticles.size(); j++)
    {
      for (unsigned int k = 0; k < photonKinematicParticles.size(); k++)
      {
        hBsMassFromP4->Fill((recoMatchedMuons[i]->p4() + recoMatchedMuons[j]->p4() + recoMatchedPhotons[k]->p4()).M());
        
        RefCountedKinematicParticle mu1 = muonKinematicParticles.at(i);
        RefCountedKinematicParticle mu2 = muonKinematicParticles.at(j);
        RefCountedKinematicParticle pho = photonKinematicParticles.at(k);
        std::vector<RefCountedKinematicParticle> allParticles;
        allParticles.push_back(mu1);
        allParticles.push_back(mu2);
        allParticles.push_back(pho);


        GlobalVector initialPhotonMomentum = pho->currentState().kinematicParameters().momentum();
        const ParticleMass bs_mass = 5.366;

        GlobalPoint pvGlobalPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());

        KinematicParticleVertexFitter fitter;
        cout << "Fitting" << endl;
        RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);

        if (!vertexFitTree->isValid()) continue;
        // get the fitted particle and vertex
        vertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
        RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();
        if (!fitVertex->vertexIsValid()) continue;

        GlobalPoint fittedGlobalPoint = fitVertex->position();
        reco::Candidate::Point genPoint = genMuons[0]->vertex();
        reco::Candidate::Point fittedPoint(fittedGlobalPoint.x(), fittedGlobalPoint.y(), fittedGlobalPoint.z());

        GlobalVector PVToSV = fittedGlobalPoint - pvGlobalPoint;
        GlobalVector BsMomentum = fitParticle->currentState().kinematicParameters().momentum();
        hCosSimBsVsSV->Fill(acos(BsMomentum.dot(PVToSV) / (BsMomentum.mag() * PVToSV.mag()))*180./3.14159);

        hBsMassResidual->Fill(fitParticle->currentState().mass() - bs_mass);
        hXResidual->Fill(fittedGlobalPoint.x() - genSV.x());
        hZResidual->Fill(fittedGlobalPoint.z() - genSV.z());
        hChisquaredProb->Fill(TMath::Prob(fitVertex->chiSquared(), fitVertex->degreesOfFreedom()));


        /////////////////////
        // try global fit
        /////////////////////
        KinematicParticleFactoryFromTransientTrack pFactory;
        vector<RefCountedKinematicParticle> allParticlesGlobal;
        vertexFitTree->movePointerToTheTop();
        vertexFitTree->movePointerToTheFirstChild();
        RefCountedKinematicParticle mu1Global = vertexFitTree->currentParticle();
        FreeTrajectoryState fts(mu1Global->currentState().globalPosition(), mu1Global->currentState().globalMomentum(), mu1Global->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 muonCov;
        AlgebraicSymMatrix77 muonCov77 = mu1Global->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            muonCov(i, j) = muonCov77(i, j);
          }
        }
        CartesianTrajectoryError muonErr(muonCov);
        fts.setCartesianError(muonErr);
        reco::TransientTrack muonTTGlobal = theB->build(fts);
        float sigma = 0.001;
        mu1Global = pFactory.particle(muonTTGlobal, mu1Global->currentState().mass(), float(0), float(0), sigma);
        allParticlesGlobal.push_back(mu1Global);

        vertexFitTree->movePointerToTheNextChild();
        RefCountedKinematicParticle mu2Global = vertexFitTree->currentParticle();
        FreeTrajectoryState fts2(mu2Global->currentState().globalPosition(), mu2Global->currentState().globalMomentum(), mu2Global->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 muonCov2;
        AlgebraicSymMatrix77 muonCov77_2 = mu2Global->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            muonCov2(i, j) = muonCov77_2(i, j);
          }
        }
        CartesianTrajectoryError muonErr2(muonCov2);
        fts2.setCartesianError(muonErr2);
        reco::TransientTrack muonTTGlobal2 = theB->build(fts2);
        mu2Global = pFactory.particle(muonTTGlobal2, mu2Global->currentState().mass(), float(0), float(0), sigma);
        allParticlesGlobal.push_back(mu2Global);

        vertexFitTree->movePointerToTheNextChild();
        RefCountedKinematicParticle phoGlobal = vertexFitTree->currentParticle();
        FreeTrajectoryState fts3(phoGlobal->currentState().globalPosition(), phoGlobal->currentState().globalMomentum(), phoGlobal->currentState().particleCharge(), &field);
        AlgebraicSymMatrix66 photonCov;
        AlgebraicSymMatrix77 photonCov77 = phoGlobal->currentState().kinematicParametersError().matrix();
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 6; j++)
          {
            photonCov(i, j) = photonCov77(i, j);
          }
        }
        CartesianTrajectoryError photonErr(photonCov);
        fts3.setCartesianError(photonErr);
        reco::TransientTrack phoTTGlobal = theB->build(fts3);
        float sigma3 = 0.001;
        phoGlobal = pFactory.particle(phoTTGlobal, phoGlobal->currentState().mass(), float(0), float(0), sigma3);

        allParticlesGlobal.push_back(phoGlobal);

        vertexFitTree->movePointerToTheTop();

        // create the constraint
        MultiTrackKinematicConstraint* multiPointingConstraint = new MultiTrackPointingKinematicConstraint(genPV);
        KinematicConstrainedVertexFitter constrainedFitter;
        RefCountedKinematicTree vertexFitTreeGlobal = constrainedFitter.fit(allParticlesGlobal, multiPointingConstraint);
        if (!vertexFitTreeGlobal->isValid()) continue;
        // get the fitted particle and vertex
        vertexFitTreeGlobal->movePointerToTheTop();
        RefCountedKinematicParticle fitParticleGlobal = vertexFitTreeGlobal->currentParticle();
        RefCountedKinematicVertex fitVertexGlobal = vertexFitTreeGlobal->currentDecayVertex();
        if (!fitVertexGlobal->vertexIsValid()) continue;
        GlobalPoint fittedGlobalPointGlobal = fitVertexGlobal->position();
        reco::Candidate::Point fittedPointGlobal(fittedGlobalPointGlobal.x(), fittedGlobalPointGlobal.y(), fittedGlobalPointGlobal.z());
        // reco::Candidate::Point genPointGlobal = genMuons[0]->vertex();
        reco::Candidate::Point pvPointGlobal(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());

        // invariant mass
        hBsMassGlobal->Fill(fitParticleGlobal->currentState().mass());

        hBsMassResidualGlobal->Fill(fitParticleGlobal->currentState().mass() - 5.366);
        hXResidualGlobal->Fill(fittedGlobalPointGlobal.x() - genSV.x());
        hZResidualGlobal->Fill(fittedGlobalPointGlobal.z() - genSV.z());
        hChisquaredProbGlobal->Fill(ROOT::Math::chisquared_cdf_c(fitVertexGlobal->chiSquared(), fitVertexGlobal->degreesOfFreedom()));

        // angle between the bs momentum and the line from the primary vertex to the secondary vertex
        GlobalVector PVToSVGlobal = fittedGlobalPointGlobal - pvGlobalPoint;
        GlobalVector BsMomentumGlobal = fitParticleGlobal->currentState().kinematicParameters().momentum();
        hMomSVPVAngle->Fill(acos(BsMomentumGlobal.dot(PVToSVGlobal) / (BsMomentumGlobal.mag() * PVToSVGlobal.mag()))*180./3.14159);

        hDistanceGlobal->Fill((fittedPointGlobal - genPoint).R());
        hDistanceFromPVGlobal->Fill((fittedPointGlobal - pvPointGlobal).R());

        // resolution
        hMuonPtResReco->Fill((genMatchedMuons[i]->pt() - recoMatchedMuons[i]->pt())/genMatchedMuons[i]->pt());
        hMuonPtResReco->Fill((genMatchedMuons[j]->pt() - recoMatchedMuons[j]->pt())/genMatchedMuons[j]->pt());
        hPhotonPtResReco->Fill((genMatchedPhotons[k]->pt() - recoMatchedPhotons[k]->pt())/genMatchedPhotons[k]->pt());

        hPhotonEnergyResReco->Fill((genMatchedPhotons[k]->energy() - recoMatchedPhotons[k]->energy())/genMatchedPhotons[k]->energy());

        vertexFitTreeGlobal->movePointerToTheFirstChild();
        RefCountedKinematicParticle fittedMu1 = vertexFitTreeGlobal->currentParticle();
        hMuonPtResFit->Fill((genMatchedMuons[i]->pt() - fittedMu1->currentState().kinematicParameters().momentum().perp())/genMatchedMuons[i]->pt());
        vertexFitTreeGlobal->movePointerToTheNextChild();
        RefCountedKinematicParticle fittedMu2 = vertexFitTreeGlobal->currentParticle();
        hMuonPtResFit->Fill((genMatchedMuons[j]->pt() - fittedMu2->currentState().kinematicParameters().momentum().perp())/genMatchedMuons[j]->pt());
        vertexFitTreeGlobal->movePointerToTheNextChild();
        RefCountedKinematicParticle fittedPho = vertexFitTreeGlobal->currentParticle();
        hPhotonPtResFit->Fill((genMatchedPhotons[k]->pt() - fittedPho->currentState().kinematicParameters().momentum().perp())/genMatchedPhotons[k]->pt());
        hPhotonEnergyResFit->Fill((genMatchedPhotons[k]->energy() - fittedPho->currentState().kinematicParameters().energy())/genMatchedPhotons[k]->energy());
        hPhotonEnergyRecoVsFit->Fill(recoMatchedPhotons[k]->energy() - fittedPho->currentState().kinematicParameters().energy());


        //// end of global fit
        ////
        ////

        // pointing constraint sequential fit
        //
        KinematicConstraint* pointingConstraint = new PointingKinematicConstraint(genPV);
        KinematicParticleFitter kinematicFitter;
        vertexFitTree = kinematicFitter.fit(pointingConstraint, vertexFitTree);
        if (!vertexFitTree->isValid()) continue;

        // get the fitted particle and vertex
        vertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fitParticlePointing = vertexFitTree->currentParticle();
        RefCountedKinematicVertex fitVertexPointing = vertexFitTree->currentDecayVertex();
        if (!fitVertexPointing->vertexIsValid()) continue;

        GlobalPoint fittedGlobalPointPointing = fitVertexPointing->position();
        reco::Candidate::Point fittedPointPointing(fittedGlobalPointPointing.x(), fittedGlobalPointPointing.y(), fittedGlobalPointPointing.z());

        hNormalVsPointingDistance->Fill((fittedPoint - fittedPointPointing).R());

        // invariant mass
        hBsMass->Fill(fitParticle->currentState().mass());
        hBsMassPointing->Fill(fitParticlePointing->currentState().mass());

        // now do the same but use only the muons
        std::vector<RefCountedKinematicParticle> muonParticles;
        muonParticles.push_back(mu1);
        muonParticles.push_back(mu2);
        KinematicParticleVertexFitter muonFitter;
        RefCountedKinematicTree muonVertexFitTree = muonFitter.fit(muonParticles);
        if (!muonVertexFitTree->isValid()) continue;
        muonVertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle muonFitParticle = muonVertexFitTree->currentParticle();
        RefCountedKinematicVertex muonFitVertex = muonVertexFitTree->currentDecayVertex();
        if (!muonFitVertex->vertexIsValid()) continue;

        GlobalPoint muonFittedGlobalPoint = muonFitVertex->position();
        reco::Candidate::Point muonFittedPoint(muonFittedGlobalPoint.x(), muonFittedGlobalPoint.y(), muonFittedGlobalPoint.z());
        reco::Candidate::Point muonGenPoint(genMuons[0]->vertex().x(), genMuons[0]->vertex().y(), genMuons[0]->vertex().z());
        //

        hDimuonVertexVsMMGVertexDistance->Fill((muonFittedPoint - fittedPointGlobal).R());
        hDimuonDistance->Fill((muonFittedPoint - genPoint).R());


        h2vs3Distance->Fill((muonFittedPoint - fittedPoint).R());

        hDistance->Fill((fittedPoint - genPoint).R());
        hDistancePointing->Fill((fittedPointPointing - genPoint).R());

        // distance from primary vertex
        if(primaryVertices.size() > 0)
        {
          reco::Candidate::Point pvPoint(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
          hDistanceFromPV->Fill((fittedPoint - pvPoint).R());
        }

        // cosine similarity between initial and refitted photon momentum
        vertexFitTree->movePointerToTheFirstChild();
        vertexFitTree->movePointerToTheNextChild();
        vertexFitTree->movePointerToTheNextChild();
        if(vertexFitTree->currentParticle()->currentState().mass() == 0)
        {
          RefCountedKinematicParticle refittedPhoton = vertexFitTree->currentParticle();
          GlobalVector refittedPhotonMomentum = refittedPhoton->currentState().kinematicParameters().momentum();
          double cosineSimilarity = refittedPhotonMomentum.dot(initialPhotonMomentum) / (refittedPhotonMomentum.mag() * initialPhotonMomentum.mag());
          hPhotonCosineSimilarity->Fill(cosineSimilarity);

        }
      }
    }
  }

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(Test);

