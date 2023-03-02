/**********************************************************************************************/ /**
*  Class that computes the attenuation between a GS and a satellite due to clouds. 
*  @class      PropagationLossModelITUR
*  @author     Arnau Dolz Puig (ADP), arnau.dolz@estudiantat.upc.edu
*  @date       2021-nov-25
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
***************************************************************************************************/

#include "Clouds.hpp"

LOG_COMPONENT_DEFINE("Clouds");

Clouds::Clouds(double temp)
    : m_temp(temp)
{}

void Clouds::setCloud()
{
    double fog[] = {0.7, 0.05};
    m_h_cloud = fog[0];
    m_wlc     = fog[1];
}

bool Clouds::isValid(ECICoordinates body1, ECICoordinates body2)
{
    /* Checks if both bodies are the same. */
    if((body1.x == body2.x) && (body1.y == body2.y) && (body1.z == body2.z)) {
        return false;
    }

    double down_bound = (10 * (Globals::constants.pi / 180));
    ns3::Vector src = CoordinateSystemUtils::fromECIToNS3Vector(body1);
    ns3::Vector dest = CoordinateSystemUtils::fromECIToNS3Vector(body2);
    double src_norm = src.GetLength();

    /* Find the angle between the two points */
    ns3::Vector diff_vec = dest - src;
    double diff_vec_norm = diff_vec.GetLength();
    ns3::Vector diff_norm_vec(diff_vec.x / diff_vec_norm,
        diff_vec.y / diff_vec_norm, diff_vec.z / diff_vec_norm);
    ns3::Vector src_norm_vec(src.x / src_norm,
        src.y / src_norm, src.z / src_norm);
    double dot_res = diff_norm_vec.x * src_norm_vec.x +
        diff_norm_vec.y * src_norm_vec.y + diff_norm_vec.z * src_norm_vec.z;
    m_angle = std::abs(Globals::constants.pi/2 - std::acos(std::fabs(dot_res)));

    /* Checks if the angle is inside the accpeted range. */
    if(m_angle > down_bound) {
        return true;
    } else {
        return false;
    }
}

void Clouds::getDistanceGsSat()
{
    m_dist_travel = m_h_cloud / std::sin(m_angle);

}

double Clouds::getAttCoeff(double freq)
{
    double k1 = 0;         /**< Benoit's extintion coeficient [dB/Km]/[g/m³]. */
    k1 = std::pow(freq, BENOIT_CONSTANT_A1) * 
         std::exp(BENOIT_CONSTANT_A2 * (1 + BENOIT_CONSTANT_A3 * m_temp)); 
    return k1;
}

double Clouds::getExtCoeff(double k1)
{
    double kex = 0;    /**< Extintion coeficient [dB/Km] */
    kex = k1 * m_wlc;
    return kex;
}

void Clouds::getCloudsAttdB(
    ns3::Ptr<SpaceNetDevice> src,
    ECICoordinates body1,
    ECICoordinates body2,
    double min_freq
)
{
    double k1, kex, att;
    bool visivility;    
    m_src = src;
    m_freq = m_src->getFrequency();

    /* Setting up of the cloud parameters. */
    setCloud();
    /* Setting up the minimum frequency to perform the model. */
    setMinFrequency(min_freq);
    /* Check the angle of visivility to be high enough. */
    visivility = isValid(body1, body2);

    /* Check if there is no visibility and if the device frequency is high enough. */
    if(visivility == false || src->getFrequency() < m_freq_min) {
        m_att = 0;
    } else {
        k1  = getAttCoeff(src->getFrequency());
        kex = getExtCoeff(k1);
        getDistanceGsSat();
        att =  kex * m_dist_travel;
        m_att = att;
    }
}

double Clouds::DoCalcRxPower(
    double tx_power,
    ns3::Ptr<ns3::MobilityModel> src,
    ns3::Ptr<ns3::MobilityModel> dest
) const
{
    ECICoordinates src_pos = CoordinateSystemUtils::fromNS3VectorToECI(src->GetPosition());
    ECICoordinates dest_pos = CoordinateSystemUtils::fromNS3VectorToECI(dest->GetPosition());
    double result = tx_power - getAtt();
    return result;
}
