﻿#ifndef AUTOCOMPLETEGRAPH_ACG_LOCALIZATION_07012018
#define AUTOCOMPLETEGRAPH_ACG_LOCALIZATION_07012018


#include <cmath>
#include "auto_complete_graph/ACG.hpp"
#include "auto_complete_graph/VertexAndEdge/EdgeLocalization.hpp"
#include "auto_complete_graph/VertexAndEdge/EdgePriorObservation.hpp"
#include "auto_complete_graph/VertexAndEdge/VertexSE2RobotLocalization.hpp"
//#include "graph_map/graph_map.h"
#include "auto_complete_graph/Localization/Localization.hpp"
#include "auto_complete_graph/Localization/LocalizationConvertion.hpp"

//Needed for registraiton between submaps
#include <ndt_registration/ndt_matcher_d2d_2d.h>
#include <ndt_registration/ndt_matcher_d2d.h>


#include "auto_complete_graph/GraphMapLocalizationMsg.h"

namespace AASS {

namespace acg{	

    class AutoCompleteGraphLocalization : public AutoCompleteGraphBase<AutoCompleteGraphPriorXY, g2o::VertexXYPrior, g2o::EdgeXYPriorACG>{
        protected:

	    double _number_of_links_to_prior = 0;

	    ///@brief register the submaps
	    bool _do_own_registration;
	    ///@brief Extract the corners from the submaps
	    bool _extract_corners;
	    ///@brief Use the gaussian of the corner approximated by ndt_feature_finder
	    bool _use_corner_covariance;
	    ///@brief Use the covariance of MCL into the link between corner in submaps and prior. NOT WORKING :)
		bool _use_covariance_for_links;
	    ///@brief Add an observation between mcl poses and prior corner if the prior is close enough to a NDT corner. The obervation is the corner observation from the robot to the NDT corner.
	    bool _use_mcl_observation_on_prior;
	    ///@brief Add link in between ndt corner and prior corner based on a distance threshold.
	    bool _use_links_prior_classic_ssrr;
	    ///@brief when adding obersvation between the prior and mcl, this is the threshold of the score for creating an observation. Score depend on mcl cov and distance between corners.
	    double _threshold_of_score_for_creating_a_link;
	    ///@brief do we want to scale the score a little bit to help out ?
	    double _scaling_factor_gaussian;

	    bool _use_robot_maps;

	    bool _use_mcl_cov_to_find_prior_observed;

	    std::vector<g2o::EdgeLocalization*> _edges_localization;

	    ///@brief prior observation
	    std::vector<g2o::EdgePriorObservation*> _edge_prior_observation;
	    std::vector<g2o::VertexSE2RobotLocalization*> _nodes_localization;
	    g2o::VertexXYPrior* _vertex_reference_for_montecarlo;
        
        
        public:
        AutoCompleteGraphLocalization(const g2o::SE2& sensoffset, 
						const Eigen::Vector2d& tn, 
						double rn,
						const Eigen::Vector2d& ln,
						const Eigen::Vector2d& pn,
						double rp,
						ndt_feature::NDTFeatureGraph* ndt_graph
  					) : _do_own_registration(true), _extract_corners(true), _use_corner_covariance(true), _use_covariance_for_links(false), _use_mcl_observation_on_prior(true), _scaling_factor_gaussian(1), _threshold_of_score_for_creating_a_link(0.5), _use_links_prior_classic_ssrr(false), _use_robot_maps(true), _use_mcl_cov_to_find_prior_observed(false), AutoCompleteGraphBase<AutoCompleteGraphPriorXY, g2o::VertexXYPrior, g2o::EdgeXYPriorACG>(sensoffset, tn, rn, ln, pn, rp, ndt_graph){

	        if(_use_links_prior_classic_ssrr && _use_mcl_observation_on_prior){
		        throw std::runtime_error("ATTENTION: you used some funny parameters here young padawan. Link prior and MCL Observation edge on prior together, will lead to the prior being linked in two different ways. Take care.");
	        }
        }
  		
  		AutoCompleteGraphLocalization(const g2o::SE2& sensoffset, 
						  const Eigen::Vector2d& tn, 
						  double rn,
						  const Eigen::Vector2d& ln,
						  const Eigen::Vector2d& pn,
						  double rp
					) : _do_own_registration(true), _extract_corners(true), _use_corner_covariance(true), _use_covariance_for_links(false), _use_mcl_observation_on_prior(true), _scaling_factor_gaussian(1), _threshold_of_score_for_creating_a_link(0.5), _use_links_prior_classic_ssrr(false), _use_robot_maps(true), _use_mcl_cov_to_find_prior_observed(false), AutoCompleteGraphBase<AutoCompleteGraphPriorXY, g2o::VertexXYPrior, g2o::EdgeXYPriorACG>(sensoffset, tn, rn, ln, pn, rp){

		    if(_use_links_prior_classic_ssrr && _use_mcl_observation_on_prior){
			    throw std::runtime_error("ATTENTION: you used some funny parameters here young padawan. Link prior and MCL Observation edge on prior together, will lead to the prior being linked in two different ways. Take care.");
		    }

	    }
		
		AutoCompleteGraphLocalization(const g2o::SE2& sensoffset, const std::string& load_file) : _do_own_registration(true), _extract_corners(true), _use_corner_covariance(true), _use_covariance_for_links(false), _use_mcl_observation_on_prior(true), _scaling_factor_gaussian(1), _threshold_of_score_for_creating_a_link(0.5), _use_links_prior_classic_ssrr(false), _use_robot_maps(true), _use_mcl_cov_to_find_prior_observed(false), AutoCompleteGraphBase<AutoCompleteGraphPriorXY, g2o::VertexXYPrior, g2o::EdgeXYPriorACG>(sensoffset, load_file){

			if(_use_links_prior_classic_ssrr && _use_mcl_observation_on_prior){
				throw std::runtime_error("ATTENTION: you used some funny parameters here young padawan. Link prior and MCL Observation edge on prior together, will lead to the prior being linked in two different ways. Take care.");
			}

		}

	    void print() const{
			AutoCompleteGraphBase<AutoCompleteGraphPriorXY, g2o::VertexXYPrior, g2o::EdgeXYPriorACG>::print();
		    std::cout << "Localization parameters: " << std::endl;

		    std::cout << "Number of localization nodes: " << _nodes_localization.size() << std::endl;

		    std::cout << "Use robot maps: " <<  _use_robot_maps << std::endl;
		    ///@brief register the submaps
		    std::cout << "Do own registration: " << _do_own_registration << std::endl;
		    ///@brief Extract the corners from the submaps
		    std::cout << "Extract corners: " << _extract_corners << std::endl;
		    ///@brief Use the gaussian of the corner approximated by ndt_feature_finder
		    std::cout << "Use corner covariance: " << _use_corner_covariance << std::endl;
		    ///@brief Use the covariance of MCL into the link between corner in submaps and prior. NOT WORKING :)
		    std::cout << "Use MCL covariance in links: " << _use_covariance_for_links << std::endl;
		    ///@brief Add an observation between mcl poses and prior corner if the prior is close enough to a NDT corner. The obervation is the corner observation from the robot to the NDT corner.
		    std::cout << "Use MCL corner observatino to directly link to the prior: " << _use_mcl_observation_on_prior << std::endl;
		    ///@brief Add link in between ndt corner and prior corner based on a distance threshold.
		    std::cout << "Use SSRR link strategy: " << _use_links_prior_classic_ssrr << std::endl;
		    ///@brief when adding obersvation between the prior and mcl, this is the threshold of the score for creating an observation. Score depend on mcl cov and distance between corners.
		    std::cout << "Threshold for creating a link using MCL covariance: " << _threshold_of_score_for_creating_a_link << std::endl;
		    ///@brief do we want to scale the score a little bit to help out ?
		    std::cout << "Scaling factor for gaussians: " << _scaling_factor_gaussian << std::endl;

		    std::cout << "Number of links to prior: " << _number_of_links_to_prior << std::endl;
	    }

	    virtual bool checkAbleToOptimize(){
//		    if(_number_of_links_to_prior > 0) {
			    return true;
//		    }
//		    return false;
	    }

	    void setFirst(){
		    g2o::OptimizableGraph::Vertex *fRobot;
		    if(_nodes_localization.size() > 0) {
			    fRobot = _nodes_localization[0];
		    }
		    else {
			    fRobot = *(_prior->getNodes().begin());
		    }
		    _optimizable_graph.setFirst(fRobot);
	    }

	    void doOwnRegistrationBetweenSubmaps(bool setter){_do_own_registration = setter;}
	    void extractCorners(bool setter){_extract_corners = setter;}
	    void useCornerCovariance(bool setter){ _use_corner_covariance = setter;}
	    void useCovarianceToFindLinks(bool setter){ _use_covariance_for_links = setter;}
	    void setScalingFactorOfGaussians(double setter){_scaling_factor_gaussian = setter;}
	    void setThrehsoldOfScoreForCreatingLink(double setter){_threshold_of_score_for_creating_a_link = setter;}
	    void useMCLObservationOnPrior(bool setter){_use_mcl_observation_on_prior = setter;}
	    void useLinksPriorSSRR(bool setter){_use_links_prior_classic_ssrr = setter;}
	    void useRobotMaps(bool setter){_use_robot_maps = setter;}
	    void useMCLCovToFindPriorObserved(bool setter){_use_mcl_cov_to_find_prior_observed = setter;}

	    std::vector<g2o::EdgeLocalization*>& getLocalizationEdges(){return _edges_localization;}
	    const std::vector<g2o::EdgeLocalization*>& getLocalizationEdges() const {return _edges_localization;}
	    std::vector<g2o::EdgePriorObservation*>& getPriorObservations(){return _edge_prior_observation;}
	    const std::vector<g2o::EdgePriorObservation*>& getPriorObservations() const {return  _edge_prior_observation;}
	    std::vector<g2o::VertexSE2RobotLocalization*>& getRobotPoseLocalization(){return _nodes_localization;}
	    const std::vector<g2o::VertexSE2RobotLocalization*>& getRobotPoseLocalization() const {return _nodes_localization;}

	    /***FUNCTIONS TO ADD THE NODES***/
	    g2o::VertexSE2RobotLocalization* addRobotLocalization(const g2o::SE2& se2_robot_pose, const Eigen::Affine3d& affine_original_robot_pose, Eigen::Vector3d to_robot_localization, const Eigen::Matrix3d& cov_localization, const std::shared_ptr< perception_oru::NDTMap >& map);
	    g2o::VertexSE2RobotLocalization* addRobotLocalization(const Eigen::Vector3d& rob_localization, const Eigen::Affine3d& affine_original_robot_pose, Eigen::Vector3d to_robot_localization, const Eigen::Matrix3d& cov_localization, const std::shared_ptr< perception_oru::NDTMap >& map);
	    g2o::VertexSE2RobotLocalization* addRobotLocalization(double x, double y, double theta, const Eigen::Affine3d& affine_original_robot_pose, Eigen::Vector3d to_robot_localization, const Eigen::Matrix3d& cov_localization, const std::shared_ptr< perception_oru::NDTMap >& map);
		
		/** FUNCTION TO ADD THE EGDES **/
		g2o::EdgeLocalization* addLocalization(const g2o::SE2& localization, g2o::HyperGraph::Vertex* v1, const Eigen::Matrix3d& information);
		g2o::EdgeLocalization* addLocalization(const g2o::SE2& localization, int from_id, const Eigen::Matrix3d& information);
		g2o::EdgeLocalization* addLocalization(double x, double y, double theta, int from_id, const Eigen::Matrix3d& information);

	    virtual g2o::EdgePriorObservation* addPriorObservation(const g2o::Vector2& pos, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2, const Eigen::Matrix2d& covariance_landmark, g2o::EdgeLandmark_malcolm* equivalent_landmark_observation_edge);
	    virtual g2o::EdgePriorObservation* addPriorObservation(const g2o::Vector2& pos, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2, g2o::EdgeLandmark_malcolm* equivalent_landmark_observation_edge);
	    virtual g2o::EdgePriorObservation* addPriorObservation(const g2o::Vector2& pos, int from_id, int toward_id, g2o::EdgeLandmark_malcolm* equivalent_landmark_observation_edge);

	    /**
	     * @brief Uses the covariance from MCL instead of a user chosen one.
	     * @param pos link size
	     * @param v2 from
	     * @param v1 toward
	     * @return link edge created
	     */
	    g2o::EdgeLinkXY_malcolm* addLinkBetweenMaps(const g2o::Vector2& pos, g2o::VertexXYPrior* v2, g2o::VertexLandmarkNDT* v1, const g2o::VertexSE2RobotLocalization* mcl_pose);
// 		g2o::EdgeLocalization* addLocalization(const g2o::SE2& observ, int from_id);
// 		g2o::EdgeLocalization* addLocalization(double x, double y, double theta, int from_id);
		
		/** Others **/
		/**@brief set the vertex in the prior that is going to be the reference point for the localization. return NULL if failed, otherwise return the pointer to the vertex that was chosen
		**/
		g2o::VertexXYPrior* setPriorReference();

		/**
		 * @brief Incrementally update the NDTGraph UPDATED TO THE NEW VERSION :)
		 * Localization and the new nodes are added to the graph. If the g2o graph has 4 nodes, only nodes 5 to last node of the NDT graph are added to it.
		 * Add NDT-corners and Robot poses.
		 */
		void updateNDTGraph(const auto_complete_graph::GraphMapLocalizationMsg& ndt_graph_localization);

		/**
		 * @brief Incrementally update the NDTGraph UPDATED TO THE NEW VERSION :)
		 * All new nodes and new localization are added to the graph. Old information is not touched.
		 */
		void addNDTGraph(const auto_complete_graph::GraphMapLocalizationMsg& ndt_graph_localization);


		std::tuple<g2o::VertexSE2RobotLocalization*, std::shared_ptr<perception_oru::NDTMap> > addElementNDT(const auto_complete_graph::GraphMapLocalizationMsg& ndt_graph_localization, int element);

		void extractCornerNDTMap(const std::shared_ptr< perception_oru::NDTMap >& map, g2o::VertexSE2RobotLocalization* robot_localization);

		/**
		 * Add all new localization edges
		 * @param ndt_graph_localization
		 * @param element
		 * @param robot_ptr
		 */
		g2o::VertexSE2RobotLocalization* addLocalizationVertex(
				const auto_complete_graph::GraphMapLocalizationMsg &ndt_graph_localization, int element,
				const std::shared_ptr<perception_oru::NDTMap> &shared_map, const g2o::SE2& robot_pose);

		std::tuple<Eigen::Affine3d, Eigen::MatrixXd> registerSubmaps(const g2o::VertexSE2RobotPose& from,
		                                                             const g2o::VertexSE2RobotPose& toward,
		                                                             Eigen::Affine3d &transformation,
		                                                             int nb_neighbor);


		/**
		 * Returns all the corner found in the NDT map in NDTCornerGraphElement. The corner positions are in the global frame while the obervation are in the robot pose frame, as in needed by g2o
		 */
		void getAllCornersNDTTranslatedToGlobalAndRobotFrame(const std::shared_ptr<perception_oru::NDTMap>& map, g2o::VertexSE2RobotPose* robot_ptr, std::vector<AASS::acg::NDTCornerGraphElement>& corners_end);



		virtual void testInfoNonNul(const std::string& before = "no data") const ;


		void AddObservationsMCLPrior();

//		void addObservationMCLToPrior(const g2o::VertexLandmarkNDT* landmark) const;
		void addObservationMCLToPrior(const g2o::VertexLandmarkNDT* landmark);


		/**
		 * Update all measurements in prior observation and create new observation
		 */
		void updatePriorObservations();

		/**
		 * @brief update all prior observation ot have the same measurment as the equivalent landmark observation from the mcl pose.
		 */
		void updateExistingPriorObservations();


//		int createNewLinks();
    };

}

}


#endif
