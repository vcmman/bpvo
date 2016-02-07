#ifndef BPVO_POSE_ESTIMATOR_GN_H
#define BPVO_POSE_ESTIMATOR_GN_H

#include <bpvo/pose_estimator_base.h>
#include <bpvo/debug.h>
#include <bpvo/linear_system_builder.h>
#include <Eigen/Cholesky>

namespace bpvo {

template <class TDataT>
class PoseEstimatorGN : public PoseEstimatorBase< PoseEstimatorGN<TDataT> >
{
 public:
  typedef PoseEstimatorGN<TDataT> Self;
  typedef PoseEstimatorBase<Self> Base;

  using typename Base::Warp;
  using typename Base::Jacobian;
  using typename Base::Gradient;
  using typename Base::ParameterVector;
  using typename Base::Hessian;
  using typename Base::TemplateData;
  using typename Base::Channels;
  using typename Base::PoseEstimatorData;

 public:
  /**
   * \param params pose estimation parameters
   */
  PoseEstimatorGN(const PoseEstimatorParameters& params = PoseEstimatorParameters())
  {
    Base::setParameters(params);
  }

  /**
   * \return the )ame of the algorithm
   */
  inline std::string name() const { return "PoseEstimatorGN"; }


  /**
   * \param tdata       template data
   * \param channels    input channels
   * \param pose        current pose
   * \param H           hessian [optional]
   * \param G           gradien [optional]
   * \return            functionv value (norm of the residuals vector)
   */
  inline float linearize(TemplateData* tdata, const Channels& channels, PoseEstimatorData& data)
  {
    tdata->computeResiduals(channels, data.T, Base::residuals(), Base::valid());
    auto sigma = this->_scale_estimator.estimateScale(Base::residuals(), Base::valid());
    computeWeights(this->_params.lossFunction, Base::residuals(), Base::valid(), sigma, Base::weights());

    this->_num_fun_evals += 1;
    return LinearSystemBuilder::Run(
        tdata->jacobians(), Base::residuals(), Base::weights(), Base::valid(), &data.H, &data.G);
  }

  inline bool runIteration(TemplateData* tdata, const Channels& channels,
                           PoseEstimatorData& data, float& f_norm,
                           PoseEstimationStatus& status)

  {
    f_norm = this->linearize(tdata, channels, data);

    if(!data.solve()) {
      if(this->_params.verbosity != VerbosityType::kSilent)
        Warn("solver failed");
      status = PoseEstimationStatus::kSolverError;
      return false;
    }

    return true;
  }
}; // PoseEstimatorGN

}; // bpvo

#endif // BPVO_POSE_ESTIMATOR_GN_H