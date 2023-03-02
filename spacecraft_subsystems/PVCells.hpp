/***********************************************************************************************//**
 *  Class that implements a PV Cell as energy harvester for a node.
 *  @class      Energy
 *  @author     Antonio Romero Aguirre (ARA), antonio.romero@i2cat.net
 *  @date       2021-feb-10
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

#ifndef __PVCELLS_HPP__
#define __PVCELLS_HPP__

/* Global libraries */
#include "dss.hpp"

/* Internal libraries */
#include "MathUtils.hpp"

/* NS3 libraries */
#include "ns3/energy-harvester.h"

/***********************************************************************************************//**
 * Implementation of a PV Cell that inherts the EnergyHarvester frmae from NS-3. The calculation
 * of the output current of the cell (and therefore for the outut power) is done using a simple 
 * model where the equation that defines the behaviour of a PV Cell is modeled by two linear 
 * equations, one before the MPP (Maximum Power Point) and the other one afer taht point. 
 **************************************************************************************************/

class PVCells : public ns3::EnergyHarvester
{
public:

    /*******************************************************************************************//**
     * Constructor of the PV cell
     * 
     * @param Imax     Maximum current that the cell can deliver (also known as Isc).
     * @param Pmax     Maximum power that the cell can deliver (Maximum Power Point).
     * @param V_Pmax   Volatge when the cells delivers the maximum power.
     * @param Vmax     Maximum voltage of the cell (also known as Voc).
     **********************************************************************************************/
    PVCells(double Imax, double Pmax, double V_Pmax, double Vmax);

    /*******************************************************************************************//**
     *  Auto-generated destructor.
     **********************************************************************************************/ 
    ~PVCells(void) = default;

    /*******************************************************************************************//**
     * Method that gets the output current of the cell. 
     * 
     * @returns   Output current of the cell in Amperes.
     **********************************************************************************************/
    double getOutputCurrent(void) { return m_harvestingCurrent; }

    /*******************************************************************************************//**
     * Method that gets the power current of the cell.
     * 
     * @returns   Output power of the cell in Watts.
     **********************************************************************************************/
    double getOutputPower(void) { return m_harvestingPower; }

    /*******************************************************************************************//**
     * Method that updates the values of the current and the power of the cell. 
     * 
     * @param Vin    Input voltage 
     **********************************************************************************************/
    void updateInputVoltage(double Vin);

protected:
    double m_harvestingPower;   /**< Output current of the PV Cell */
    double m_harvestingCurrent; /**< Output power of the PV Cell */
    double m_Imax;              /**< Maximum current that the cell can deliver (also known as Isc) */
    double m_Pmax;              /**< Maximum power that the cell can deliver (Maximum Power Point) */
    double m_Vmax;              /**< Volatge when the cells delivers the maximum power */
    double m_V_Pmax;            /**< Maximum voltage of the cell (also known as Voc) */
    double m_Vin;               /**< Input voltage of the cell */

    /*******************************************************************************************//**
     * Method that calculates the harvesting current of the given an input voltage. If the input 
     * is invalid, it is set to 0.
     * 
     * @param Vin    Input voltage
     **********************************************************************************************/
    double calculateHarvestingCurrent(double Vin);

    /*******************************************************************************************//**
     * Method that retrives the current output power of the cell.
     * 
     * @returns Current output power of the cell.
     **********************************************************************************************/
    double DoGetPower(void) const;


};

#endif /* __PVCELLS_HPP__ */
