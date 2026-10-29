#include "RecoVertex/KinematicFitPrimitives/interface/ParticleKinematicLinearizedTrackState.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParameters.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicRefittedTrackState.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicPerigeeConversions.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicState.h"

const AlgebraicVector6& ParticleKinematicLinearizedTrackState::constantTerm() const {
  if (!jacobiansAvailable)
    computeJacobians();
  return theConstantTerm;
}

const AlgebraicMatrix63& ParticleKinematicLinearizedTrackState::positionJacobian() const {
  if (!jacobiansAvailable)
    computeJacobians();
  return thePositionJacobian;
}

const AlgebraicMatrix64& ParticleKinematicLinearizedTrackState::momentumJacobian() const {
  if (!jacobiansAvailable)
    computeJacobians();
  return theMomentumJacobian;
}

const AlgebraicVector6& ParticleKinematicLinearizedTrackState::parametersFromExpansion() const {
  if (!jacobiansAvailable)
    computeJacobians();
  return theExpandedParams;
}

AlgebraicVector6 ParticleKinematicLinearizedTrackState::predictedStateParameters() const {
  if (!jacobiansAvailable)
    computeJacobians();

  // photon: the measurement is the cluster position (two coordinates, the pivot
  // one is eliminated), the cluster energy and the mass hypothesis
  if (part->getPhoton() != nullptr) {
    int ci, cj;
    photonMeasuredIndices(ci, cj);
    const auto& pos = part->getPhoton()->caloPosition();
    const double m[3] = {pos.x(), pos.y(), pos.z()};

    AlgebraicVector6 z;
    z(0) = m[ci];
    z(1) = m[cj];
    z(2) = part->getPhoton()->energy();
    z(3) = part->currentState().mass();
    return z;
  }
  return thePredState.perigeeParameters().vector();
}

AlgebraicSymMatrix66 ParticleKinematicLinearizedTrackState::predictedStateWeight(int& error) const {
  if (!jacobiansAvailable)
    computeJacobians();

  if (part->getPhoton() != nullptr) {
    int i = 0;
    AlgebraicSymMatrix66 z = thePhotonMeasurementError.Inverse(i);
    error = i;
    return z;
  }

  int i = 0;
  AlgebraicSymMatrix66 z = thePredState.perigeeError().weightMatrix(i);
  error = i;
  return z;
  //   return thePredState.perigeeError().weightMatrix(error);
}

AlgebraicSymMatrix66 ParticleKinematicLinearizedTrackState::predictedStateError() const {
  if (!jacobiansAvailable)
    computeJacobians();

  if (part->getPhoton() != nullptr)
    return thePhotonMeasurementError;

  return thePredState.perigeeError().covarianceMatrix();
}

// ImpactPointMeasurement  ParticleKinematicLinearizedTrackState::impactPointMeasurement() const
// { throw VertexException(" ParticleKinematicLinearizedTrackState::impact point measurement is not implemented for kinematic classes!");}

TrackCharge ParticleKinematicLinearizedTrackState::charge() const { return part->initialState().particleCharge(); }

RefCountedKinematicParticle ParticleKinematicLinearizedTrackState::particle() const { return part; }

bool ParticleKinematicLinearizedTrackState::operator==(const LinearizedTrackState<6>& other) const {
  const ParticleKinematicLinearizedTrackState* otherP =
      dynamic_cast<const ParticleKinematicLinearizedTrackState*>(&other);
  if (otherP == nullptr) {
    throw VertexException(
        " ParticleKinematicLinearizedTrackState:: don't know how to compare myself to non-kinematic track state");
  }
  return (*(otherP->particle()) == *part);
}

bool ParticleKinematicLinearizedTrackState::hasError() const {
  if (!jacobiansAvailable)
    computeJacobians();
  return thePredState.isValid();
}

// here we make a and b matrices of
// our measurement function expansion.
// marices will be almost the same as for
// classical perigee, but bigger:
// (6x3) and (6x4) respectivelly.
void ParticleKinematicLinearizedTrackState::computeJacobians() const {
  GlobalPoint paramPt(theLinPoint);
  thePredState = builder(part->currentState(), paramPt);
  //  bool valid = thePredState.isValid();
  //  if (!valid) std::cout <<"Help!!!!!!!!! State is invalid\n";
  //  if (!valid) return;
  if (part->getPhoton() != nullptr) {
    // photon
    computePhotonJacobians();
  }
  else if (std::abs(theCharge) < 1e-5) {
    //neutral track
    computeNeutralJacobians();
  } else {
    //charged track
    computeChargedJacobians();
  }
  jacobiansAvailable = true;
}

ReferenceCountingPointer<LinearizedTrackState<6> >
ParticleKinematicLinearizedTrackState::stateWithNewLinearizationPoint(const GlobalPoint& newLP) const {
  RefCountedKinematicParticle pr = part;
  return new ParticleKinematicLinearizedTrackState(newLP, pr);
}

ParticleKinematicLinearizedTrackState::RefCountedRefittedTrackState
ParticleKinematicLinearizedTrackState::createRefittedTrackState(const GlobalPoint& vertexPosition,
                                                                const AlgebraicVector4& vectorParameters,
                                                                const AlgebraicSymMatrix77& covarianceMatrix) const {
  KinematicPerigeeConversions conversions;
  KinematicState lst = conversions.kinematicState(
      vectorParameters, vertexPosition, charge(), covarianceMatrix, part->currentState().magneticField());
  RefCountedRefittedTrackState rst =
      RefCountedRefittedTrackState(new KinematicRefittedTrackState(lst, vectorParameters));
  return rst;
}

AlgebraicVector4 ParticleKinematicLinearizedTrackState::predictedStateMomentumParameters() const {
  if (!jacobiansAvailable)
    computeJacobians();
  AlgebraicVector4 res;
  res[0] = thePredState.perigeeParameters().vector()(0);
  res[1] = thePredState.perigeeParameters().vector()(1);
  res[2] = thePredState.perigeeParameters().vector()(2);
  res[3] = thePredState.perigeeParameters().vector()(5);
  return res;
}

AlgebraicSymMatrix44 ParticleKinematicLinearizedTrackState::predictedStateMomentumError() const {
  int error;
  if (!jacobiansAvailable)
    computeJacobians();
  AlgebraicSymMatrix44 res;
  AlgebraicSymMatrix33 m3 = thePredState.perigeeError().weightMatrix(error).Sub<AlgebraicSymMatrix33>(0, 0);
  res.Place_at(m3, 0, 0);
  res(3, 0) = thePredState.perigeeError().weightMatrix(error)(5, 5);
  res(3, 1) = thePredState.perigeeError().weightMatrix(error)(5, 0);
  res(3, 2) = thePredState.perigeeError().weightMatrix(error)(5, 1);
  res(3, 3) = thePredState.perigeeError().weightMatrix(error)(5, 2);
  return res;
}

double ParticleKinematicLinearizedTrackState::weightInMixture() const { return 1.; }

std::vector<ReferenceCountingPointer<LinearizedTrackState<6> > > ParticleKinematicLinearizedTrackState::components()
    const {
  std::vector<ReferenceCountingPointer<LinearizedTrackState<6> > > res;
  res.reserve(1);
  res.push_back(RefCountedLinearizedTrackState(const_cast<ParticleKinematicLinearizedTrackState*>(this)));
  return res;
}

AlgebraicVector6 ParticleKinematicLinearizedTrackState::refittedParamFromEquation(
    const RefCountedRefittedTrackState& theRefittedState) const {
  AlgebraicVectorM momentum = theRefittedState->momentumVector();
  if ((momentum(2) * predictedStateMomentumParameters()(2) < 0) && (std::fabs(momentum(2)) > M_PI / 2)) {
    if (predictedStateMomentumParameters()(2) < 0.)
      momentum(2) -= 2 * M_PI;
    if (predictedStateMomentumParameters()(2) > 0.)
      momentum(2) += 2 * M_PI;
  }

  AlgebraicVector3 vertexPosition;
  vertexPosition(0) = theRefittedState->position().x();
  vertexPosition(1) = theRefittedState->position().y();
  vertexPosition(2) = theRefittedState->position().z();
  AlgebraicVector6 param = constantTerm() + positionJacobian() * vertexPosition + momentumJacobian() * momentum;

  // for photons param(2) is the energy, not an angle
  checkParameters(param);

  return param;
}

void ParticleKinematicLinearizedTrackState::checkParameters(AlgebraicVector6& parameters) const {
  if (part->getPhoton() == nullptr)
  {
    if (parameters(2) > M_PI)
      parameters(2) -= 2 * M_PI;
    if (parameters(2) < -M_PI)
      parameters(2) += 2 * M_PI;
  }
}

void ParticleKinematicLinearizedTrackState::computeChargedJacobians() const {
  GlobalPoint paramPt(theLinPoint);

  double field = part->currentState().magneticField()->inInverseGeV(thePredState.theState().globalPosition()).z();
  double signTC = -part->currentState().particleCharge();

  double thetaAtEP = thePredState.theState().globalMomentum().theta();
  double phiAtEP = thePredState.theState().globalMomentum().phi();
  double ptAtEP = thePredState.theState().globalMomentum().perp();
  double transverseCurvatureAtEP = field / ptAtEP * signTC;

  // Fix calculation for case where magnetic field swaps sign between previous state and current state
  if (field * part->currentState().magneticField()->inInverseGeV(part->currentState().globalPosition()).z() < 0.) {
    signTC = -signTC;
  }

  double x_v = thePredState.theState().globalPosition().x();
  double y_v = thePredState.theState().globalPosition().y();
  double z_v = thePredState.theState().globalPosition().z();
  double X = x_v - paramPt.x() - sin(phiAtEP) / transverseCurvatureAtEP;
  double Y = y_v - paramPt.y() + cos(phiAtEP) / transverseCurvatureAtEP;
  double SS = X * X + Y * Y;
  double S = sqrt(SS);

  // The track parameters at the expansion point
  theExpandedParams[0] = transverseCurvatureAtEP;
  theExpandedParams[1] = thetaAtEP;
  theExpandedParams[3] = 1 / transverseCurvatureAtEP - signTC * S;

  theExpandedParams[5] = part->currentState().mass();

  double phiFEP;
  if (std::abs(X) > std::abs(Y)) {
    double signX = (X > 0.0 ? +1.0 : -1.0);
    phiFEP = -signTC * signX * acos(signTC * Y / S);
  } else {
    phiFEP = asin(-signTC * X / S);
    if ((signTC * Y) < 0.0)
      phiFEP = M_PI - phiFEP;
  }
  if (phiFEP > M_PI)
    phiFEP -= 2 * M_PI;
  theExpandedParams[2] = phiFEP;
  theExpandedParams[4] =
      z_v - paramPt.z() - (phiAtEP - theExpandedParams[2]) / tan(thetaAtEP) / transverseCurvatureAtEP;

  thePositionJacobian(2, 0) = -Y / (SS);
  thePositionJacobian(2, 1) = X / (SS);
  thePositionJacobian(3, 0) = -signTC * X / S;
  thePositionJacobian(3, 1) = -signTC * Y / S;
  thePositionJacobian(4, 0) = thePositionJacobian(2, 0) / tan(thetaAtEP) / transverseCurvatureAtEP;
  thePositionJacobian(4, 1) = thePositionJacobian(2, 1) / tan(thetaAtEP) / transverseCurvatureAtEP;
  thePositionJacobian(4, 2) = 1;

  //debug code - to be removed later
  //   cout<<"parameters for momentum jacobian: X "<<X<<endl;
  //   cout<<"parameters for momentum jacobian: Y "<<Y<<endl;
  //   cout<<"parameters for momentum jacobian: SS "<<SS<<endl;
  //   cout<<"parameters for momentum jacobian: PhiAtEP "<<phiAtEP<<endl;
  //   cout<<"parameters for momentum jacobian: curv "<<transverseCurvatureAtEP<<endl;
  //   cout<<"sin phi Atep "<<sin(phiAtEP)<<endl;
  //   cout<<"cos phi at EP "<<cos(phiAtEP)<<endl;
  //   cout<<"upper  part is "<<X*cos(phiAtEP) + Y*sin(phiAtEP) <<endl;
  //   cout<<"lower part is"<<SS*transverseCurvatureAtEP*transverseCurvatureAtEP<<endl;

  theMomentumJacobian(0, 0) = 1;
  theMomentumJacobian(1, 1) = 1;

  theMomentumJacobian(2, 0) =
      -(X * cos(phiAtEP) + Y * sin(phiAtEP)) / (SS * transverseCurvatureAtEP * transverseCurvatureAtEP);

  theMomentumJacobian(2, 2) = (Y * cos(phiAtEP) - X * sin(phiAtEP)) / (SS * transverseCurvatureAtEP);

  theMomentumJacobian(3, 0) =
      (signTC * (Y * cos(phiAtEP) - X * sin(phiAtEP)) / S - 1) / (transverseCurvatureAtEP * transverseCurvatureAtEP);

  theMomentumJacobian(3, 2) = signTC * (X * cos(phiAtEP) + Y * sin(phiAtEP)) / (S * transverseCurvatureAtEP);

  theMomentumJacobian(4, 0) =
      (phiAtEP - theExpandedParams(2)) / tan(thetaAtEP) / (transverseCurvatureAtEP * transverseCurvatureAtEP) +
      theMomentumJacobian(2, 0) / tan(thetaAtEP) / transverseCurvatureAtEP;

  theMomentumJacobian(4, 1) =
      (phiAtEP - theExpandedParams(2)) * (1 + 1 / (tan(thetaAtEP) * tan(thetaAtEP))) / transverseCurvatureAtEP;

  theMomentumJacobian(4, 2) = (theMomentumJacobian(2, 2) - 1) / tan(thetaAtEP) / transverseCurvatureAtEP;

  theMomentumJacobian(5, 3) = 1;

  // And finally the residuals:
  AlgebraicVector3 expansionPoint;
  expansionPoint[0] = thePredState.theState().globalPosition().x();
  expansionPoint[1] = thePredState.theState().globalPosition().y();
  expansionPoint[2] = thePredState.theState().globalPosition().z();
  AlgebraicVector4 momentumAtExpansionPoint;
  momentumAtExpansionPoint[0] = transverseCurvatureAtEP;  // Transverse Curv
  momentumAtExpansionPoint[1] = thetaAtEP;
  momentumAtExpansionPoint[2] = phiAtEP;
  momentumAtExpansionPoint[3] = theExpandedParams[5];

  theConstantTerm = AlgebraicVector6(theExpandedParams - thePositionJacobian * expansionPoint -
                                     theMomentumJacobian * momentumAtExpansionPoint);
}

void ParticleKinematicLinearizedTrackState::computeNeutralJacobians() const {
  GlobalPoint paramPt(theLinPoint);
  double thetaAtEP = thePredState.theState().globalMomentum().theta();
  double phiAtEP = thePredState.theState().globalMomentum().phi();
  double ptAtEP = thePredState.theState().globalMomentum().perp();

  double x_v = thePredState.theState().globalPosition().x();
  double y_v = thePredState.theState().globalPosition().y();
  double z_v = thePredState.theState().globalPosition().z();
  double X = x_v - paramPt.x();
  double Y = y_v - paramPt.y();

  // The track parameters at the expansion point
  theExpandedParams[0] = 1 / ptAtEP;
  theExpandedParams[1] = thetaAtEP;
  theExpandedParams[2] = phiAtEP;
  theExpandedParams[3] = X * sin(phiAtEP) - Y * cos(phiAtEP);
  theExpandedParams[4] = z_v - paramPt.z() - (X * cos(phiAtEP) + Y * sin(phiAtEP)) / tan(thetaAtEP);
  theExpandedParams[5] = part->currentState().mass();

  // The Jacobian: (all at the expansion point)
  // [i,j]
  // i = 0: rho = 1/pt , 1: theta, 2: phi_p, 3: epsilon, 4: z_p
  // j = 0: x_v, 1: y_v, 2: z_v
  thePositionJacobian(3, 0) = sin(phiAtEP);
  thePositionJacobian(3, 1) = -cos(phiAtEP);
  thePositionJacobian(4, 0) = -cos(phiAtEP) / tan(thetaAtEP);
  thePositionJacobian(4, 1) = -sin(phiAtEP) / tan(thetaAtEP);
  thePositionJacobian(4, 2) = 1;

  // [i,j]
  // i = 0: rho = 1/pt , 1: theta, 2: phi_p, 3: epsilon, 4: z_p
  // j = 0: rho = 1/pt , 1: theta, 2: phi_v
  theMomentumJacobian(0, 0) = 1;
  theMomentumJacobian(1, 1) = 1;
  theMomentumJacobian(2, 2) = 1;

  theMomentumJacobian(3, 2) = X * cos(phiAtEP) + Y * sin(phiAtEP);
  theMomentumJacobian(4, 1) = theMomentumJacobian(3, 2) * (1 + 1 / (tan(thetaAtEP) * tan(thetaAtEP)));

  theMomentumJacobian(4, 2) = (X * sin(phiAtEP) - Y * cos(phiAtEP)) / tan(thetaAtEP);
  theMomentumJacobian(5, 3) = 1;

  // And finally the residuals:
  AlgebraicVector3 expansionPoint;
  expansionPoint[0] = thePredState.theState().globalPosition().x();
  expansionPoint[1] = thePredState.theState().globalPosition().y();
  expansionPoint[2] = thePredState.theState().globalPosition().z();
  AlgebraicVector4 momentumAtExpansionPoint;
  momentumAtExpansionPoint[0] = 1 / ptAtEP;
  momentumAtExpansionPoint[1] = thetaAtEP;
  momentumAtExpansionPoint[2] = phiAtEP;
  momentumAtExpansionPoint[3] = theExpandedParams[5];

  theConstantTerm = AlgebraicVector6(theExpandedParams - thePositionJacobian * expansionPoint -
                                     theMomentumJacobian * momentumAtExpansionPoint);
}

void ParticleKinematicLinearizedTrackState::photonMeasuredIndices(int& i, int& j) const {
  i = (thePhotonPivot == 0) ? 1 : 0;
  j = (thePhotonPivot == 2) ? 1 : 2;
}

// Photon measurement model, following BaBar (physics/0503191, Eqs. 38-40) and
// Belle II (NIM A 976 (2020) 164269, Eqs. 19-21): the photon flies on a straight
// line from the vertex v to the cluster m, m = v + tau * p. The flight length tau
// is eliminated with the coordinate k of the largest momentum component (pivot):
//   h_i = v_i + (m_k - v_k) * p_i / p_k
//   h_j = v_j + (m_k - v_k) * p_j / p_k
//   h_E = sqrt(p^2 + m^2)
//   h_m = m
// The momentum is parametrised as for neutral particles, q = (rho = 1/pt, theta, phi, m),
// so that the refitted states and FinalTreeBuilder can treat the photon like any other track.
// Rows 4 and 5 are unused (zero Jacobians, tiny variance).
void ParticleKinematicLinearizedTrackState::computePhotonJacobians() const {
  const GlobalPoint vtx = thePredState.theState().globalPosition();
  const GlobalVector mom = thePredState.theState().globalMomentum();
  const double v[3] = {vtx.x(), vtx.y(), vtx.z()};
  const double p[3] = {mom.x(), mom.y(), mom.z()};
  const double mass = part->currentState().mass();

  const auto& pos = part->getPhoton()->caloPosition();
  const double m[3] = {pos.x(), pos.y(), pos.z()};

  // pivot: largest momentum component, so that p_i/p_k and p_j/p_k stay bounded
  thePhotonPivot = 0;
  for (int c = 1; c < 3; ++c) {
    if (std::abs(p[c]) > std::abs(p[thePhotonPivot]))
      thePhotonPivot = c;
  }
  const int k = thePhotonPivot;
  int i, j;
  photonMeasuredIndices(i, j);

  const double ri = p[i] / p[k];
  const double rj = p[j] / p[k];
  const double d = m[k] - v[k];
  const double energy = sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + mass * mass);

  // The measurement function at the expansion point
  theExpandedParams = AlgebraicVector6();
  theExpandedParams[0] = v[i] + d * ri;
  theExpandedParams[1] = v[j] + d * rj;
  theExpandedParams[2] = energy;
  theExpandedParams[3] = mass;

  // [row, column]: rows = (h_i, h_j, h_E, h_m), columns = (x_v, y_v, z_v)
  thePositionJacobian = AlgebraicMatrix63();
  thePositionJacobian(0, i) = 1;
  thePositionJacobian(0, k) = -ri;
  thePositionJacobian(1, j) = 1;
  thePositionJacobian(1, k) = -rj;

  // derivatives with respect to the cartesian momentum (px, py, pz, m)
  AlgebraicMatrix44 cartesianJacobian;
  cartesianJacobian(0, i) = d / p[k];
  cartesianJacobian(0, k) = -d * ri / p[k];
  cartesianJacobian(1, j) = d / p[k];
  cartesianJacobian(1, k) = -d * rj / p[k];
  for (int c = 0; c < 3; ++c)
    cartesianJacobian(2, c) = p[c] / energy;
  cartesianJacobian(2, 3) = mass / energy;
  cartesianJacobian(3, 3) = 1;

  // d(px, py, pz, m) / d(rho, theta, phi, m) with px = cos(phi)/rho, py = sin(phi)/rho, pz = 1/(rho tan(theta))
  const double rho = 1. / mom.perp();
  const double theta = mom.theta();
  const double phi = mom.phi();
  AlgebraicMatrix44 cartesianFromPerigee;
  for (int c = 0; c < 3; ++c)
    cartesianFromPerigee(c, 0) = -p[c] / rho;
  cartesianFromPerigee(2, 1) = -1. / (rho * sin(theta) * sin(theta));
  cartesianFromPerigee(0, 2) = -p[1];
  cartesianFromPerigee(1, 2) = p[0];
  cartesianFromPerigee(3, 3) = 1;

  const AlgebraicMatrix44 momentumJacobian = cartesianJacobian * cartesianFromPerigee;
  theMomentumJacobian = AlgebraicMatrix64();
  theMomentumJacobian.Place_at(momentumJacobian, 0, 0);

  // Covariance of the reduced measurement (BaBar Eq. 40): V' = P V P^T, with P the
  // derivative of the residuals r_i = m_i - h_i, r_j = m_j - h_j with respect to the
  // cluster position (m_x, m_y, m_z), so that the uncertainty of m_k is included.
  const TMatrixD& clusterCov = *(part->getPhotonCov());
  AlgebraicSymMatrix33 positionCov;
  for (int a = 0; a < 3; ++a) {
    for (int b = 0; b <= a; ++b)
      positionCov(a, b) = clusterCov(a, b);
  }
  AlgebraicMatrix23 projection;
  projection(0, i) = 1;
  projection(0, k) = -ri;
  projection(1, j) = 1;
  projection(1, k) = -rj;
  const AlgebraicSymMatrix22 reducedPositionCov = ROOT::Math::Similarity(projection, positionCov);

  // energy uncertainty matching the energy returned by reco::Photon::energy()
  const reco::Photon* photon = part->getPhoton();
  const reco::Photon::P4type p4type = photon->getCandidateP4type() == reco::Photon::undefined
                                          ? reco::Photon::ecal_photons
                                          : photon->getCandidateP4type();
  const double energyError = photon->getCorrectedEnergyError(p4type);

  thePhotonMeasurementError = AlgebraicSymMatrix66();
  thePhotonMeasurementError.Place_at(reducedPositionCov, 0, 0);
  thePhotonMeasurementError(2, 2) = energyError * energyError;
  thePhotonMeasurementError(3, 3) = 1e-5;
  thePhotonMeasurementError(4, 4) = 1e-5;
  thePhotonMeasurementError(5, 5) = 1e-5;

  // And finally the residuals:
  AlgebraicVector3 expansionPoint(v[0], v[1], v[2]);
  AlgebraicVector4 momentumAtExpansionPoint(rho, theta, phi, mass);

  theConstantTerm = AlgebraicVector6(theExpandedParams - thePositionJacobian * expansionPoint -
                                     theMomentumJacobian * momentumAtExpansionPoint);
}

reco::TransientTrack ParticleKinematicLinearizedTrackState::track() const {
  throw VertexException(" ParticleKinematicLinearizedTrackState:: no TransientTrack to return");
}
