/**********************************************************************************************//**
 *  Class that computes the attenuation between a GS and a satellite due to clouds.
 *  @class      Clouds
 *  @author     Arnau Dolz (AD), arnau.dolz@estudiantat.upc.edu
 *  @date       2022-jan-4
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 *************************************************************************************************/

#ifndef __CLOUDS_HPP__
#define __CLOUDS_HPP__

/* Global libraries */
#include "dss.hpp"

/* External libraries */
#include <ns3/propagation-loss-model.h>

/* Internal libraries */
#include "SpaceNetDevice.hpp"
#include "CoordinateSystemUtils.hpp"

#define BENOIT_CONSTANT_A1 1.95      /**< Constant from Benoit's empirical expression. */
#define BENOIT_CONSTANT_A2 -6.866    /**< Constant from Benoit's empirical expression. */
#define BENOIT_CONSTANT_A3 4.5e-3    /**< Constant from Benoit's empirical expression. */

/**********************************************************************************************//**
 *  Attenuation in the link between a ground station and a body_2 (and viceversa) caused by clouds.
 *  It requires the distance between the GS and the body_2.
 *************************************************************************************************/
class Clouds : public ns3::PropagationLossModel
{
public:
    /******************************************************************************************//**
     *  Contructor that sets the main parameters to the clouds class such as the minimum frequency
     *  to allow the model work and the temperature of the clouds (which is going to be considered
     *  equal in all cases).
     *
     * @param temp    temperature of the cloud.
     *********************************************************************************************/
    Clouds(double temp);
    
    /******************************************************************************************//**
     * Auto-generated destructor. This is made virtual to prevent derived classes to be
     *  destructed by calling this destructor through a pointer to Clouds.
     *********************************************************************************************/
    ~Clouds(void) = default;
    
    /******************************************************************************************//**
     *  Define the attenuation in dB caused by the clouds and fog (depending on the WLC), as a 
     *  function of the distance, the frquency, the water liquid content and the temperature (of
     *  the clouds). The model used to implement this class is the Benoit's empirical expression
     *  (eq. 5.106 from Microwave Remote Sensing Active and Passive, Fawwaz T. Ulabi).
     *  According to the sources, this expresion is only valid for those frequencies from 3 GHz to
     *  30 GHz, and also, only valid for those contacts wiith a Nadir less than 10º.
     *  Humidity units: kg/m³ https://www.aqua-calc.com/calculate/humidity
     *  Liquid Water Content for several clouds: https://en.wikipedia.org/wiki/Liquid_water_content
     *  
     *  @param src         SpaceNetdevice used to know the frequency.
     *  @param body_1      ECICoordinates of the fisrt body.
     *  @param body_2      ECICOordinates of the second body.
     *  @param min_freq    Minimum frequency to allow the method to be useful.
     *********************************************************************************************/
    void getCloudsAttdB(
        ns3::Ptr<SpaceNetDevice> src,
        ECICoordinates body1,
        ECICoordinates body2,
        double min_freq
    );

    /******************************************************************************************//**
     *  Inheritated class from ns3::PropagationLossModel. Is used if the model uses objects type 
     *  ns3::RandomVariableStream, set the stream numbers to the integers starting with the offset
     *  'stream'.
     *
     * @param stream    Stream that must be returned.
     *********************************************************************************************/
    virtual int64_t DoAssignStreams(int64_t stream) {return stream = 0;}

    /******************************************************************************************//**
     *  Retrieves the attenuation computed by the model.
     *
     *  @param src      Mobility model of the source device
     *  @param dest     Mobility model of the destination device
     *  @return         The attenuation du to clouds.
     *********************************************************************************************/
    double getAtt() const { return m_att; }

private:
    double m_wlc;                   /**< Water Liquid Content. */
    double m_h_cloud;               /**< Height of the cloud. */
    double m_dist_travel;           /**< Distance that the signal travels inside a cloud. */
    double m_temp;                  /**< Temperature of the water drops. */
    double m_freq_min;              /**< Minimum frequency to allow the model to work. */
    double m_angle;                 /**< Angle between 2 points. */
    double m_freq;                  /**< Working frequency */
    double m_att;                   /**< Attenuation due to clouds. */
    ns3::Ptr<SpaceNetDevice> m_src; /**< SpaceNetDevice of the source */

    /******************************************************************************************//**
     *  Set the minimum frequency of the communication.
     * 
     *  @param min_freq Minimum frequency to be defined in MHz.
     *********************************************************************************************/
    void setMinFrequency(double min_freq) { m_freq_min = min_freq; }

    /******************************************************************************************//**
     *  Set the water liquid content and the thickness of the cloud to compute the attenuation.
     *********************************************************************************************/
    void setCloud();

    /******************************************************************************************//**
     *  Retrieves the the condition to perform the model. It checks if the angle between two points
     *  is lower than 170º and higher 10º. Note that it is assumed that the line of signt between 2
     *  objects has been checked in the contact.
     * 
     *  @param body_1    Ground station position in ECI
     *  @param body_2    Satellite position in ECI 
     *  @return True if the model is valid, else otherwise.
     *********************************************************************************************/
    bool isValid(ECICoordinates body1, ECICoordinates body2);

    /******************************************************************************************//**
     *  Compute the distance that the signal is going to travel inside the cloud in the case of one
     *  satellite communicating with a ground station.
     *********************************************************************************************/
    void getDistanceGsSat();

    /******************************************************************************************//**
     * Computes and retrieves the attenuation coefficient (K1) according to benoits method.
     * 
     * @param freq    Frequency of the device.
     * @return        The value of the attenuation coefficent in [dB/Km]/[g/m³]
     *********************************************************************************************/
    double getAttCoeff(double freq);

    /******************************************************************************************//**
     * Compute and retrieves  the extintion coefficent (Kext) due to the clouds.
     * 
     *  @param k1    Attenuation coefficient
     *  @return      The value of the extintion coefficent in [dB/Km]
     *********************************************************************************************/
    double getExtCoeff(double k1);


    /******************************************************************************************//**
     * Computes and retrieves the power received by a source when the cloudss attenuation affects 
     * on the communications channel.
     * 
     *  @param rx_power Power of the transmisor device
     *  @param src      Mobility model of the source device
     *  @param dest     Mobility model of the destination device
     *  @return         The power once the atteunation has been added to the transmitted signal.
     *********************************************************************************************/
    double DoCalcRxPower(double tx_power,
        ns3::Ptr<ns3::MobilityModel> src,
        ns3::Ptr<ns3::MobilityModel> dest
        ) const;
};

#endif  /* __CLOUDS_HPP__ */