/***********************************************************************************************//**
 *  Class that implements a PV Cell as enregy harvester for a node.
 *  @class      Energy
 *  @author     Antonio Romero Aguirre (ARA), antonio.romero@i2cat.net
 *  @author     Joan Adrià Ruiz-de-Azúa (JARA), joan.adria@tsc.upc.edu
 *  @date       2021-feb-10
 *  @copyright  This file is part of a project developed at Nano-Satellite and Payload Laboratory
 *              (NanoSat Lab), Universitat Politècnica de Catalunya - UPC BarcelonaTech.
 **************************************************************************************************/

#include "PVCells.hpp"

#include "ns3/assert.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

LOG_COMPONENT_DEFINE("PVCells");


PVCells::PVCells(double Imax, double Pmax, double V_Pmax, double Vmax)
    : m_Imax(Imax)
    , m_Pmax(Pmax)
    , m_V_Pmax(V_Pmax)
    , m_Vmax(Vmax)
{}

double PVCells::calculateHarvestingCurrent(double Vin)
{   
    m_Vin = Vin;
    double Iout;
    if(m_Vin < 0 || m_Vin > m_Vmax) {

        std::stringstream ss;
        ss << "Invalid Vin = " << m_Vin << " V, Vin set to 0 V \n";
        LOG_WARN(ss.str());

        m_Vin = 0;
        Iout = m_Imax;
    } else if(m_Vin < m_V_Pmax) {
        Iout =
            MathUtils::calculateLinearEquationTwoPoints(m_Imax, 0, (m_Pmax / m_V_Pmax), m_V_Pmax, Vin);
    } else {
        Iout = 
            MathUtils::calculateLinearEquationTwoPoints((m_Pmax / m_V_Pmax), m_V_Pmax, 0, m_Vmax, Vin);
    }
    return Iout;
}

double PVCells::DoGetPower(void) const
{
    return m_harvestingPower;
}

void PVCells::updateInputVoltage(double Vin)
{
    m_harvestingCurrent = calculateHarvestingCurrent(Vin);
    m_harvestingPower = m_harvestingCurrent * m_Vin;
}
