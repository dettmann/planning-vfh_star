#ifndef VFHSTAR_TREESEARCH_H
#define VFHSTAR_TREESEARCH_H

#include <base/pose.h>
#include <base/waypoint.h>
#include <base/geometry/spline.h>
#include <vector>
#include <list>
#include <base/eigen.h>

namespace vfh_star {
class TreeNode
{
    friend class Tree;
    public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	TreeNode();
	TreeNode(const base::Pose &pose, double dir);

        bool isRoot() const;
        bool isLeaf() const;
	
	const base::Pose &getPose() const;
	const TreeNode *getParent() const;
	double getDirection() const;
	int getDepth() const;

	double getCost() const;
	void setCost(double value);
	double getHeuristic() const;
	void setHeuristic(double value);
        double getHeuristicCost() const;

        double getPositionTolerance() const;
        void setPositionTolerance(double tol);
        double getHeadingTolerance() const;
        void setHeadingTolerance(double tol);
	
    private:
	TreeNode *parent;
        bool is_leaf;
	
	///pose of the node, note the orientation of the node and the direction may differ, 
	///because of kinematic constrains of the robot
	base::Pose pose;

	///direction, that was choosen, that lead to this node
	double direction;
	double cost;
	double heuristic;
	int depth;

        double positionTolerance;
        double headingTolerance;
};

class Tree
{
    public:
	Tree();
	~Tree();

        Tree(Tree const& other);
        Tree& operator = (Tree const& other);

	void addChild(TreeNode *parent, TreeNode *child);
	void removeChild(TreeNode *parent, TreeNode *child);

	TreeNode *getParent(TreeNode *child);
	TreeNode *getRootNode();

	void setRootNode(TreeNode *root);

        std::vector<base::Waypoint> buildTrajectoryTo(TreeNode const* leaf) const;

        int getSize() const;
        void clear();
        void verifyHeuristicConsistency(const TreeNode* from) const;
        std::list<TreeNode*> const& getNodes() const;

    private:
        /** A tree might get quite big, in which case calling nodes.size() is
         * really not efficient. Since it is trivial to keep the size uptodate,
         * just do it
         */
        int size; 
        std::list<TreeNode*> nodes;
};

struct TreeSearchConf {
    int maxTreeSize;
    double stepDistance; //! the distance in meters between two steps in the search
    int angularSampling; //! the maximum number of directions to be samples
    double discountFactor; //! the cost discount factor applied on the cost of nodes at depth D + 1 w.r.t. the node at depth D
    double obstacleSafetyDistance; //! the margin distance between the robot and the obstacles
    double robotWidth; //! the radius of the circle used to model the robot

    TreeSearchConf();
};

class TreeSearch
{
    public:
        typedef std::vector<double> Angles;
        typedef std::vector< std::pair<double, double> > AngleIntervals;

	TreeSearch();

        static base::geometry::Spline<3> waypointsToSpline(const std::vector<base::Waypoint>& waypoints);

        /** This is a version of getWaypoints where the returned set of
         * waypoints is converted into a spline
         */
        base::geometry::Spline<3> getTrajectory(const base::Pose& start);

        /** Computes the optimal path as a set of waypoints
         */
        std::vector<base::Waypoint> getWaypoints(const base::Pose& start);
	
	void setSearchConf(const TreeSearchConf& conf);
        const TreeSearchConf& getSearchConf() const;
        Tree const& getTree() const;
	
	virtual ~TreeSearch();

    protected:
	Angles getDirectionsFromIntervals(const AngleIntervals& intervals);

        // The tree generated at the last call to getTrajectory
        Tree tree;
        TreeSearchConf search_conf;
	
        base::Vector3d targetLinePoint;
        base::Vector3d targetLineNormal;

        /** Returns true if the given node is a terminal node, i.e. if it
         * reached the goal
         */
        virtual bool isTerminalNode(const TreeNode& node) const = 0;

        /** Returns the estimated cost from the given node to the optimal node
         * reachable from that node. Note that this estimate must be a minorant,
         * i.e. must be smaller or equal than the actual value
         */
	virtual double getHeuristic(const TreeNode &node) const = 0;

        /** Returns the cost of travelling from \c node's parent to \c node
         * itself. It might include a cost of "being at" \c node as well
         */
	virtual double getCostForNode(const TreeNode& node) const = 0;

	/**
	* This method returns possible directions where 
	* the robot can drive to, from the given position
        *
        * The returned vector is a list of angle intervals
	**/
	virtual AngleIntervals getNextPossibleDirections(
                const base::Pose &curPose,
                double obstacleSafetyDist,
                double robotWidth) const = 0;

	/**
	* This function returns a pose in which the robot would
	* be if he would have driven towards the given direction.
	* This method should take the robot driving constrains
	* into account. 
	*/
	virtual std::pair<base::Pose, bool> getProjectedPose(const base::Pose &curPose,
                double heading,
                double distance) const = 0;
};
} // vfh_star namespace

#endif // VFHSTAR_H
