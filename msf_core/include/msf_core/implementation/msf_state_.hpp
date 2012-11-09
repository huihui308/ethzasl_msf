/*
 *  Created on: Nov 7, 2012
 *      Author: slynen
 */

#include <msf_core/msf_types.tpp>
#include <msf_core/msf_tmp.hpp>
#include <msf_core/msf_statedef.hpp>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <vector>
#include <msf_core/eigen_conversions.h>
#include <sensor_fusion_comm/ExtState.h>
#include <sensor_fusion_comm/DoubleArrayStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <msf_core/msf_userdefinedcalculationbase.hpp>

namespace msf_core{

//returns the stateVar at position INDEX in the state list, non const version only for msf_core use
//you must not make these functions public. Instead const_cast the state object to const to use the overload
template<typename stateVector_T>
template<int INDEX>
inline typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type
GenericState_T<stateVector_T>::getStateVar() {

	BOOST_STATIC_ASSERT_MSG((msf_tmp::IsReferenceType<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::value),
			"Assumed that boost::fusion would return a reference type here, which is not the case");

	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_);
}

//returns the state at position INDEX in the state list, non const version
//you must not make these functions public. Instead const_cast the state object to const to use the overload
template<typename stateVector_T>
template<int INDEX>
inline typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t::value_t&
GenericState_T<stateVector_T>::get() {

	BOOST_STATIC_ASSERT_MSG((msf_tmp::IsReferenceType<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::value),
			"Assumed that boost::fusion would return a reference type here, which is not the case");

	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_).state_;
}


//apply the correction vector to all state vars
template<typename stateVector_T>
inline void GenericState_T<stateVector_T>::correct(const Eigen::Matrix<double, nErrorStatesAtCompileTime, 1>& correction) {
	boost::fusion::for_each(
			statevars_,
			msf_tmp::correctState<const Eigen::Matrix<double, nErrorStatesAtCompileTime, 1>, stateVector_T >(correction)
	);
}

//returns the Q-block of the state at position INDEX in the state list, not allowed for core states
template<typename stateVector_T>
template<int INDEX>
inline typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t::Q_T&
GenericState_T<stateVector_T>::getQBlock(){
	typedef typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t StateVar_T;

	BOOST_STATIC_ASSERT_MSG(static_cast<int>(StateVar_T::statetype_) != static_cast<int>(msf_core::CoreStateWithPropagation) ||
			static_cast<int>(StateVar_T::statetype_) != static_cast<int>(msf_core::CoreStateWithoutPropagation), "You requested a non-const reference to a Q-Block for a"
			"core state of the EKF, but this is not allowed! Use the const version to get Q-blocks for core states.");

	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_).Q_;
}



//returns the Q-block of the state at position INDEX in the state list, also possible for core states, since const
template<typename stateVector_T>
template<int INDEX>
inline const typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t::Q_T&
GenericState_T<stateVector_T>::getQBlock() const {
	typedef typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type StateVar_T;
	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_).Q_;
}


/// assembles a PoseWithCovarianceStamped message from the state
/** it does not set the header */
template<typename stateVector_T>
void GenericState_T<stateVector_T>::toPoseMsg(geometry_msgs::PoseWithCovarianceStamped & pose){ //boost fusion unfortunately doesn't like this to be const
	eigen_conversions::vector3dToPoint(get<msf_core::p_>(), pose.pose.pose.position);
	eigen_conversions::quaternionToMsg(get<msf_core::q_>(), pose.pose.pose.orientation);
	getPoseCovariance(pose.pose.covariance);
}


/// assembles an ExtState message from the state
/** it does not set the header */
template<typename stateVector_T>
void GenericState_T<stateVector_T>::toExtStateMsg(sensor_fusion_comm::ExtState & state) {
	eigen_conversions::vector3dToPoint(get<msf_core::p_>(), state.pose.position);
	eigen_conversions::quaternionToMsg(get<msf_core::q_>(), state.pose.orientation);
	eigen_conversions::vector3dToPoint(get<msf_core::v_>(), state.velocity);
}


/// assembles a DoubleArrayStamped message from the state
/** it does not set the header */
template<typename stateVector_T>
void GenericState_T<stateVector_T>::toFullStateMsg(sensor_fusion_comm::DoubleArrayStamped & state) { //boost fusion unfortunately doesn't like this to be const
	state.data.resize(nStatesAtCompileTime); //make sure this is correctly sized
	boost::fusion::for_each(
			statevars_,
			msf_tmp::FullStatetoDoubleArray<std::vector<double>, stateVector_T >(state.data)
	);
}


/// assembles a DoubleArrayStamped message from the state
/** it does not set the header */
template<typename stateVector_T>
void GenericState_T<stateVector_T>::toCoreStateMsg(sensor_fusion_comm::DoubleArrayStamped & state) {//boost fusion unfortunately doesn't like this to be const
	state.data.resize(nCoreStatesAtCompileTime); //make sure this is correctly sized
	boost::fusion::for_each(
			statevars_,
			msf_tmp::CoreStatetoDoubleArray<std::vector<double>, stateVector_T >(state.data)
	);
}

//returns the state at position INDEX in the state list, const version
template<typename stateVector_T>
template<int INDEX>
inline const typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t::value_t&
GenericState_T<stateVector_T>::get() const {

	BOOST_STATIC_ASSERT_MSG((msf_tmp::IsReferenceType<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::value),
			"Assumed that boost::fusion would return a reference type here, which is not the case");

	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_).state_;
}


template<typename stateVector_T>
template<int INDEX>
inline typename msf_tmp::AddConstReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t
GenericState_T<stateVector_T>::getStateVar() const {

	BOOST_STATIC_ASSERT_MSG((msf_tmp::IsReferenceType<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::value),
			"Assumed that boost::fusion would return a reference type here, which is not the case");

	return boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_);
}


template<typename stateVector_T>
template<int INDEX>
inline void GenericState_T<stateVector_T>::set(const typename msf_tmp::StripConstReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t::value_t& newvalue) {
	typedef typename msf_tmp::StripReference<typename boost::fusion::result_of::at_c<stateVector_T, INDEX >::type>::result_t StateVar_T;

	BOOST_STATIC_ASSERT_MSG(static_cast<int>(StateVar_T::statetype_) != static_cast<int>(msf_core::CoreStateWithPropagation) ||
			static_cast<int>(StateVar_T::statetype_) != static_cast<int>(msf_core::CoreStateWithoutPropagation), "You requested to set a new value for a"
			"core state of the EKF, but this is not allowed! This is an Error.");

	boost::fusion::at<boost::mpl::int_<INDEX> >(statevars_).state_ = newvalue;
}

/// resets the state
/**
 * 3D vectors: 0; quaternion: unit quaternion; scale: 1; time:0; Error covariance: zeros
 */
template<typename stateVector_T>
void GenericState_T<stateVector_T>::reset(boost::shared_ptr<UserDefinedCalculationBase> usercalc){

	// reset all states
	boost::fusion::for_each(
			statevars_,
			msf_tmp::resetState()
	);

	//reset system inputs
	w_m_.setZero();
	a_m_.setZero();

	q_int_.setIdentity();

	P_.setZero();
	time_ = 0;

	//now call the user provided function
	usercalc->resetState(*this);
}



/// writes the covariance corresponding to position and attitude to cov
template<typename stateVector_T>
void GenericState_T<stateVector_T>::getPoseCovariance(geometry_msgs::PoseWithCovariance::_covariance_type & cov){
	BOOST_STATIC_ASSERT(geometry_msgs::PoseWithCovariance::_covariance_type::static_size == 36);

	typedef typename msf_tmp::getEnumStateType<stateVector_T, msf_core::p_>::value p_type;
	typedef typename msf_tmp::getEnumStateType<stateVector_T, msf_core::q_>::value q_type;

	//get indices of position and attitude in the covariance matrix
	static const int idxstartcorr_p = msf_tmp::getStartIndex<stateVector_T, p_type, msf_tmp::CorrectionStateLengthForType>::value;
	static const int idxstartcorr_q = msf_tmp::getStartIndex<stateVector_T, q_type, msf_tmp::CorrectionStateLengthForType>::value;

	//TODO: remove after initial debugging
	BOOST_STATIC_ASSERT(idxstartcorr_p==0);
	BOOST_STATIC_ASSERT(idxstartcorr_q==6);

	/*        |  cov_p_p  |  cov_p_q  |
	 *        |           |           |
	 * cov =  |-----------|-----------|
	 *        |           |           |
	 *        |  cov_q_p  |  cov_q_q  |
	 */

	for (int i = 0; i < 9; i++)
		cov[i / 3 * 6 + i % 3] = P_((i / 3 + idxstartcorr_p) * nErrorStatesAtCompileTime + i % 3);

	for (int i = 0; i < 9; i++)
		cov[i / 3 * 6 + (i % 3 + 3)] = P_((i / 3 + idxstartcorr_p) * nErrorStatesAtCompileTime + (i % 3 + 6));

	for (int i = 0; i < 9; i++)
		cov[(i / 3 + 3) * 6 + i % 3] = P_((i / 3 + idxstartcorr_q) * nErrorStatesAtCompileTime + i % 3);

	for (int i = 0; i < 9; i++)
		cov[(i / 3 + 3) * 6 + (i % 3 + 3)] = P_((i / 3 + idxstartcorr_q) * nErrorStatesAtCompileTime + (i % 3 + 6));
}


};
