/***********************************************************************************************//**
 *  Class that implements a SGP4 model of orbit propagator
 *  @class      SGP4OrbitTrajectory
 *  @author     Irene Guitart Rosselló (IGR), ireneguitart87@hotmail.com
 *  @date       2022-may-22
 *  @copyright  This code has been developed by Fundació Privada Internet i Innovació Digital a 
 *              Catalunya (i2CAT). i2CAT is a non-profit research and innovation centre that 
 *              promotes mission-driven knowledge to solve business challenges, co-create solutions
 *              with a transformative impact, empower citizens through open and participative 
 *              digital social innovation with territorial capillarity, and promote pioneering and 
 *              strategic initiatives. i2CAT *aims to transfer* research project results to private 
 *              companies in order to create social and economic impact via the out-licensing of 
 *              intellectual property and the creation of spin-offs.
 *              Find more information of i2CAT projects and IP rights at:
 *              https://i2cat.net/tech-transfer/
 **************************************************************************************************/

#include "SGP4OrbitTrajectory.hpp"

LOG_COMPONENT_DEFINE("SGP4OrbitTrajectory");

SGP4OrbitTrajectory::SGP4OrbitTrajectory(
    OrbitalParams params,
    std::string sat_id,
    double init_mean_anomaly,
    bool record)
    : OrbitTrajectory(params, sat_id, record)
    , m_init_mean_anomaly(MathUtils::degToRad(init_mean_anomaly))
    , m_angular_speed(std::sqrt(Globals::constants.earth_mu / std::pow(params.semimajor_axis, 3)))
{ }

SGP4OrbitTrajectory::SGP4OrbitTrajectory(TLE tle, std::string sat_id, bool record)
    : SGP4OrbitTrajectory(tle.orbit_params, sat_id, tle.mean_anomaly, record)
{ }

SGP4OrbitTrajectory::SGP4OrbitTrajectory(ECICoordinates position)
    : OrbitTrajectory(position)
{ }

bool SGP4OrbitTrajectory::sgp4Init(
    TLE tle,
    std::string sat_id,
    gravconsttype constType,
    char opsMode
)
{ 
    double sec;
    int year, mon, day, hr, minute;
    const double xpdotp = 1440.0 / (2.0 * Globals::constants.pi);
    elsetrec satrec;

    /* First initiliaze satrec and convert units */
    satrec.satnum    = tle.sat_number;
    satrec.no_kozai  = tle.mean_motion / xpdotp;
    satrec.ecco      = tle.orbit_params.eccentricity;
    satrec.inclo     = MathUtils::degToRad(tle.orbit_params.inclination);
    satrec.nodeo     = MathUtils::degToRad(tle.orbit_params.raan);
    satrec.argpo     = MathUtils::degToRad(tle.orbit_params.arg_perigee);
    satrec.mo        = MathUtils::degToRad(tle.mean_anomaly);
    satrec.ndot      = tle.first_time / (xpdotp * 1440.0);
    satrec.nddot     = tle.second_time / (xpdotp * 1440.0 * 1440.0);
    satrec.bstar     = tle.bstar / 100000;
    satrec.elnum     = tle.tle_number;
    satrec.revnum    = tle.revolutions;
    satrec.epochyr   = tle.epoch_year;
    satrec.epochdays = tle.epoch_doy;
   
    if(satrec.epochyr < 57) {
        year = 2000 + satrec.epochyr;
    } else {
        year = 1900 + satrec.epochyr;
    }
    /* Convert TLE epoch to julian dates */
    SGP4Funcs::days2mdhms(year, satrec.epochdays, mon, day, hr, minute, sec);
    SGP4Funcs::jday(year, mon, day, hr, minute, sec, satrec.jdsatepoch, satrec.jdsatepochF);

    /* Init SGP4 parameters */
    SGP4Funcs::sgp4init(constType, opsMode, satrec.satnum , (satrec.jdsatepoch + satrec.jdsatepochF)
        - 2433281.5, satrec.bstar, satrec.ndot, satrec.nddot, satrec.ecco, satrec.argpo,
        satrec.inclo, satrec.mo, satrec.no_kozai, satrec.nodeo, satrec);

    m_satrec = satrec;

    return true;
}

std::tuple<ECICoordinates, ECICoordinates> SGP4OrbitTrajectory::sgp4Propagate(double time)
{ 
    double r[3], v[3];
    ECICoordinates eci_position;
    ECICoordinates eci_velocity;

    SGP4Funcs::sgp4(m_satrec, time, r, v);

    eci_position = ECICoordinates(r[0], r[1], r[2]);
    eci_velocity = ECICoordinates(v[0], v[1], v[2]);

    return std::make_tuple(eci_position, eci_velocity);
}

double SGP4OrbitTrajectory::computeMean(double t) const
{
    /* radians */
    return std::fmod(m_init_mean_anomaly + m_angular_speed * t, 2 * Globals::constants.pi);
}
