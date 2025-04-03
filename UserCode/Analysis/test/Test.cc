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
  TH1D* hBsMassFromP4;
  TH1D* hMuPt;
  TH1D* hGammaPt;
  TH1D* hGammaDeltaR;
  TH1D* hGammaPtWithTrigger;
  
  TH1D* hDistance;
  TH1D* hPhotonCosineSimilarity;


  int nConvPhotons = 0;
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
  hBsMassFromP4 = new TH1D("hBsMassFromP4", "hBsMassFromP4", 50, 3, 7);
  hMuPt = new TH1D("hMuPt", "hMuPt", 100, 0, 30);
  hGammaPt = new TH1D("hGammaPt", "hGammaPt", 100, 0, 30);
  hGammaDeltaR = new TH1D("hGammaDeltaR", "hGammaDeltaR", 100, 0, 0.05);
  hGammaPtWithTrigger = new TH1D("hGammaPtWithTrigger", "hGammaPtWithTrigger", 100, 0, 30);

  hDistance = new TH1D("hDistance", "hDistance", 100, 0, 0.1);
  hPhotonCosineSimilarity = new TH1D("hPhotonCosineSimilarity", "hPhotonCosineSimilarity", 100, -1, 1);

  cout << "HERE Test::beginJob()" << endl;
}

void Test::endJob()
{
  //make a new Root file
  TFile myRootFile( theConfig.getParameter<std::string>("outHist").c_str(), "RECREATE");

  //write histogram data

  hBsMass->Write();
  hBsMassFromP4->Write();
  hMuPt->Write();
  hGammaPt->Write();
  hGammaDeltaR->Write();
  hGammaPtWithTrigger->Write();

  hDistance->Write();
  hPhotonCosineSimilarity->Write();
  
  myRootFile.Close();

  delete hBsMass;
  delete hBsMassFromP4;
  delete hMuPt;
  delete hGammaPt;
  delete hGammaDeltaR;
  delete hGammaPtWithTrigger;

  delete hDistance;
  delete hPhotonCosineSimilarity;

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
      }
    }
  }

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
      recoMatchedPhotons.push_back(bestMatchedPhoton);
      genMatchedPhotons.push_back(genPh);
      // hRecoVsGenGammaPt->Fill(genPh->pt(), bestMatchedPhoton->pt());
      // hGammaPtError->Fill((bestMatchedPhoton->pt() - genPh->pt())/genPh->pt());
    }
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

    cout << "photon calo position:" << recoPho.caloPosition() << endl;
    cout << "photon energy:" << recoPho.energy() << endl;

    hGammaPt->Fill(recoPho.pt());
    GlobalPoint vtx(primaryVertices[0].position().x(), primaryVertices[0].position().y(), primaryVertices[0].position().z());
    GlobalVector p3(recoPho.px(), recoPho.py(), recoPho.pz());
    TrackCharge ch = 0;

    const ParticleMass photon_mass(0.);
    float photon_sigma = 1E-6;

    FreeTrajectoryState fts(vtx, p3, ch, &field);


    EcalClusterLazyTools lazyTools(ev, esGetTokens->get(es), ebRecHitsToken_, eeRecHitsToken_);
    TMatrixD cov(lazyTools.covariancesXYZ(*recoPho.superCluster()));
    TMatrixD* covPtr(new TMatrixD(cov));

    // cov.Print();
    
    AlgebraicSymMatrix66 photonCov{ROOT::Math::SMatrixIdentity()};
    AlgebraicVector6 diagonal(1., 1., 1., 1., 1., 1.);
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
        RefCountedKinematicParticle mu1 = muonKinematicParticles.at(i);
        RefCountedKinematicParticle mu2 = muonKinematicParticles.at(j);
        RefCountedKinematicParticle pho = photonKinematicParticles.at(k);
        std::vector<RefCountedKinematicParticle> allParticles;
        allParticles.push_back(mu1);
        allParticles.push_back(mu2);
        allParticles.push_back(pho);

        GlobalVector initialPhotonMomentum = pho->currentState().kinematicParameters().momentum();
        
        const ParticleMass bs_mass = 5.366;

        // MultiTrackKinematicConstraint* bs_mass_constraint = new MultiTrackMassKinematicConstraint(bs_mass, 3);

        KinematicParticleVertexFitter fitter;
        cout << "Fitting" << endl;
        RefCountedKinematicTree vertexFitTree = fitter.fit(allParticles);

        if (!vertexFitTree->isValid()) continue;

        vertexFitTree->movePointerToTheTop();
        RefCountedKinematicParticle fitParticle = vertexFitTree->currentParticle();
        RefCountedKinematicVertex fitVertex = vertexFitTree->currentDecayVertex();

        if (!fitVertex->vertexIsValid()) continue;

        // invariant mass
        hBsMass->Fill(fitParticle->currentState().mass());

        GlobalPoint fittedGlobalPoint = fitVertex->position();
        reco::Candidate::Point genPoint = genMuons[0]->vertex();

        reco::Candidate::Point fittedPoint(fittedGlobalPoint.x(), fittedGlobalPoint.y(), fittedGlobalPoint.z());

        hDistance->Fill((fittedPoint - genPoint).R());

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

          cout << "initial photon momentum: " << initialPhotonMomentum.x() << " " << initialPhotonMomentum.y() << " " << initialPhotonMomentum.z() << endl;
          cout << "refitted photon momentum: " << refittedPhotonMomentum.x() << " " << refittedPhotonMomentum.y() << " " << refittedPhotonMomentum.z() << endl;
        }
      }
    }
  }

  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;
}

DEFINE_FWK_MODULE(Test);

