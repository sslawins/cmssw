#include "FWCore/Framework/interface/one/EDFilter.h"

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

#include "DataFormats/Math/interface/deltaR.h"




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
class MyFilter : public edm::one::EDFilter<> {
public:

  //constructor, function is called when new object is created
  explicit MyFilter(const edm::ParameterSet& conf);

  //destructor, function is called when object is destroyed
  ~MyFilter();

  //edm MyFilter plugin specific functions
  void beginJob() override;
  bool filter(edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  bool isSameDecay(const std::vector<int>&, const std::vector<int>&);
private:

  edm::ParameterSet theConfig;
  unsigned int theEventCount;

  edm::EDGetTokenT < vector<reco::GenParticle> > theGenParticleToken;
  edm::EDGetTokenT < vector<reco::Muon> > theMuonToken;
  edm::EDGetTokenT < vector<reco::Photon> > thePhotonToken;


  std::vector<int> MuMuG = {22, 13, -13};
};


MyFilter::MyFilter(const edm::ParameterSet& conf)
  : theConfig(conf), theEventCount(0)
{
  cout <<" CTORXX" << endl;

  theMuonToken = consumes< vector<reco::Muon>  >( edm::InputTag("muons"));
  thePhotonToken = consumes< vector<reco::Photon>  >( edm::InputTag("photons"));
}

MyFilter::~MyFilter()
{
  cout <<" DTOR" << endl;
}

bool MyFilter::isSameDecay(const std::vector<int>& dec1, const std::vector<int>& dec2) {
    
    if (dec1.size() != dec2.size()) {
        return false; 
    }

    std::set<int> dec1Set(dec1.begin(), dec1.end());
    std::set<int> dec2Set(dec2.begin(), dec2.end());

    return dec1Set == dec2Set;
}


void MyFilter::beginJob()
{
  cout << "HERE MyFilter::beginJob()" << endl;
}

void MyFilter::endJob()
{
  cout << "HERE MyFilter::endJob()" << endl;
}


bool MyFilter::filter(edm::Event& ev, const edm::EventSetup& es)
{
  std::cout << " -------------------------------- HERE MyFilter::filter "<< std::endl;

  const std::vector<reco::Muon> & recoMuons = ev.get(theMuonToken);
  const std::vector<reco::Photon> & recoPhotons = ev.get(thePhotonToken);


  cout <<"*** Analyze event: " << ev.id() <<" analysed event count:" << ++theEventCount << endl;

  if(recoMuons.size() >=2 && recoPhotons.size() >= 1)
  {
    return true;
  }
  else
  {
    return false;
  }

}

DEFINE_FWK_MODULE(MyFilter);

