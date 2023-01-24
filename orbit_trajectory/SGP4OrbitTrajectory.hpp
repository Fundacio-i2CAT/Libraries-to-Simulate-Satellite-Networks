/***********************************************************************************************//**
 *  Class that implements a SGP4 model of orbit propagator
 *  @class      SGP4OrbitTrajectory
 *  @author     Irene Guitart Rosselló (IGR), ireneguitart87@hotmail.com
 *  @date       2022-may-22
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#ifndef __SGP4_ORBIT_TRAJECTORY_HPP__
#define __SGP4_ORBIT_TRAJECTORY_HPP__

/* Global libraries */
#include "dss.hpp"

/* External Libraries */
#include <cmath>
#include "SGP4.h" 

/* Internal Libraries */
#include "OrbitTrajectory.hpp"
#include "ECICoordinates.hpp"
#include "Globals.hpp"
#include "CoordinateSystemUtils.hpp"
#include "TimeUtils.hpp"

using namespace SGP4Funcs;

/***********************************************************************************************//**
 * SGP4 Satellite trajectory. Implementation of OrbitTrajectory class with the orbital SGP4 model
 *
 * @see     OrbitTrajectory
 **************************************************************************************************/
class SGP4OrbitTrajectory: public OrbitTrajectory
{
public:
    /*******************************************************************************************//**
     *  Creates a new OrbitTrajectory object that implements a SGP4 orbit propagator.
     *  Its construction is done with SGP4 orbital parameters given as floating point numbers:
     *      - Semi-major axis.
     *      - Eccentricity.
     *      - Inclination (in degrees).
     *      - Argument of the perigee (in degrees).
     *      - Right Ascension of the Ascending Node (in degrees).
     *  Additionally, the initial state can optionally be provided with one of the following
     *  parameters:
     *      - Mean anomaly (in degrees.)
     *  The initial state corresponds to the angular parameter at the start of the simulation
     *  (virtual time.) If none of the avobe are given, the initial state is assumed to be of zero
     *  degrees.
     *  If neither of the previous configuration sets are provided (or are incomplete), this
     *  function will leave all members initialized to 0.
     *
     *  @param  params              Orbital parameters.
     *  @param  sat_id              Satellite identifier.
     *  @param  init_mean_anomaly   The mean anomaly of the spacecraft at the simulation start time.
     **********************************************************************************************/
    SGP4OrbitTrajectory(
        OrbitalParams params,
        std::string sat_id,
        double init_mean_anomaly = 0,
        bool record = true
    );

    /*******************************************************************************************//**
     *  Constructs a <SGP4> propagator model taking all the orbital parameters and state from a TLE:
     *      - Initial line of a TLE (where satellite identifier is provided.)
     *      - First line of the TLE.
     *      - Second line of the TLE.
     *  If neither of the previous configuration sets are provided (or are incomplete), this
     *  function will leave all members initialized to 0.
     *
     *  @param tle          TLE object that represents the orbit of the satellite
     *  @param  sat_id      Satellite identifier.
     **********************************************************************************************/
    SGP4OrbitTrajectory(TLE tle, std::string sat_id, bool record = true);

    /*******************************************************************************************//**
     *  Constructor of a generic orbit trajectory with a specific initial position. This constructors
     *  just initializes the position of the satellite, no computation is done to retrieve orbit
     *  parameters or velocity. It is useful for testing.
     *
     *  @param  position    Initial position of the satellite
     **********************************************************************************************/
    SGP4OrbitTrajectory(ECICoordinates position);

    /*******************************************************************************************//**
    *  Auto-generated destructor. This is made virtual to prevent derived classes to be
    *  destructed by calling this destructor through a pointer to OrbitTrajectory.
    ***********************************************************************************************/
    ~SGP4OrbitTrajectory(void) = default;

    /*******************************************************************************************//**
    *  Retrieves the mean anomaly by using computeMean function. 
    ***********************************************************************************************/
    virtual double getMeanAnomaly(void) { return computeMean(TimeUtils::getSimulationTime()); }

    /*******************************************************************************************//**
    *  This function initializes the orbit propagator by reading the TLE and defining the
    *  gravitational constants and the operational mode.
    * 
    *  @param tle          TLE object that represents the orbit of the satellite
    *  @param sat_id       Satellite identifier
    *  @param constType    Sgp4 gravitational constants set type
    *  @param opsMode      Mode of operation afspc or improved ('a' or 'i')
    ***********************************************************************************************/
    bool sgp4Init(TLE tle, std::string sat_id, gravconsttype constType = wgs84, char opsMode = 'a');

    /*******************************************************************************************//**
     * Method that performs the propagation of a step. This method uses the SGP4 model to
     * propagate the satellite position.
     *
     * @param time  Simulation time in which the satellite position shall be propagated (in seconds)
     **********************************************************************************************/
    std::tuple<ECICoordinates, ECICoordinates> sgp4Propagate(double time);

protected:
    /*******************************************************************************************//**
     * Method that performs the propagation of a step. This method is related to the trajectory
     * model, therefore it is virtual. If a new model would be created, extend class and implement
     * this method.
     *
     * @param      time     time at which the position is computed (in s)
     **********************************************************************************************/
    virtual std::tuple<ECICoordinates, ECICoordinates> propagateOrbit(double time) {
        return sgp4Propagate(time);
    }

private:
    OrbitalCoordinates m_position;  /**< Satellite position in the Orbital frame. It represents thus
                                     * the radius and the true anomaly at which the satellite is
                                     * placed
                                     **/
    double m_init_mean_anomaly;     /**< Initial mean anomaly (in radians) */
    double m_angular_speed;         /**< mean angular speed (in radians/seconds) */
    elsetrec m_satrec;              /**< Embedded structure from SGP4 library which includes all
                                     * orbital parameters needed to propagate with SPG4 model
                                     **/
    
    /*******************************************************************************************//**
     *  Compute the Mean Anomaly of an orbit given the current time.
     *  @param  current_time    Current simulation time (in s).
     **********************************************************************************************/
     double computeMean(double time) const;

};

#endif /* __SGP4_ORBIT_TRAJECTORY_HPP__ */
