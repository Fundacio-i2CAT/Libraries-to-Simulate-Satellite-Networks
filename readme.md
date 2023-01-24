<img src="https://wikifab.org/images/b/b6/Group-i2CAT_logo-color-alta.jpg" width=25% height=25%>

[![Maintenance](https://img.shields.io/badge/Status-Maintained-green.svg)]()
[![Linux](https://svgshare.com/i/Zhy.svg)](https://www.linux.org/pages/download/)
[![made-with-cpp](https://img.shields.io/badge/Made%20with-C%2B%2B-blue)](https://isocpp.org/)
[![GPLv2 license](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)


# Libraries to Simulate Satellite Networks
This repositroy contains libraries that simulate satellite networks. Such libraries are compatible with the Distributed Satellite System Simulator (DSS-SIM). The description of the DSS-SIM can be found in the following paper: [Towards an Integral Model-Based Simulator for Autonomous Earth Observation Satellite Networks](https://ieeexplore.ieee.org/abstract/document/8517811). The current version of this repository allows to simulate the following aspects of the satellite networks:
* **Propagation Models**: Allows the simulation of attenuation in RF communications due to the clouds.
* **Medium Access Protocols**: Allows the simulation of the CSMA-CA including the and adapted Net Device Module.
* **Spacecraft Subsystems**: Addition of a Solar cells model.
* **Orbit Propagation**: Implements the SGP4 orbit propagation model.

# Pre-Requisites
The prerequisites to use this repository are:
* Distributed Satellite System Simulator (Contact i2CAT [here](https://i2cat.net/contact/))
* Vallado's C++ library for SGP4 (Available online [here](https://github.com/Spacecraft-Code/Vallado/tree/master/cpp/SGP4/SGP4))
* Network Simulator 3 (v3.35) (Available online [here](https://www.nsnam.org/releases/ns-3-35/))

# How to build it
This repository can not be directly build. To do so, files shall be added into its correspondent module of the DSS-SIM. After that the whole project must be build. Notice that the DSS_SIM is needed to be able to use these libraries.

# Technical Description
In order to use these libraries the previous installation of the DSS-SIM is required (As mentioned in the [Prerequisites](#pre-requisites)). Each module from this repository is independent and can be use without the others. However, the DSS-SIM follows a certain architecture, in the following table the directory in where each module shall be placed is provided. In addition, it is important to mention that the Orbit Propagator that implements SGP4, uses Vallado's algorithm to propagate the orbit. As a consequence call to the source code of this SGP4 implementation is needed. For this reason, Such files (SGP4.cpp and SGP4.h) must be place within the Orbit Propagation module.

|Developed module          |DSS-SIM Module            |
|--------------------------|--------------------------|
|Propagation Models        |Networking/Channels       |
|Medium Access Protocols   |Networking/Net_Device     |
|Spacecraft Subsystems     |Physical/Modules          |
|Orbit Propagation         |Physical/Orbit_Trajectory |

All these modules had been tested by using GTest and making unit tests for each of them before allowing them to be published. In order to use them, when preparing a simulation on the DSS-SIM, it is only needed to call functions such as: *sgp4Init* to propagate the SGP4 orbit, *getOutputPower* to obtain the output power obtained by the solar cells, *CsmaCaMacNetDevice* to create a Net Device that uses CSMA/CA, or *setCloudsPropagation* in the communications channel to retrieve the attenuation due to the clouds by means of *getAtt*. Notice that the communications channel is not provided in this repository as far as it is a part of the Distributed Satelllite System Simulator.

Finally, it is important to mention that the code in these files can be extracted to adapt it to other simulation tools if it is not desired to use DSS-SIM.

# Source
This code has been developed within the research / innovation project i2-22-RDI-IoT A2 DSS Sim. 
Aquest projecte ha rebut finançament per part del Govern de la Generalitat de Catalunya dins del marc de l'estrategia [NewSpace](https://www.accio.gencat.cat/ca/serveis/banc-coneixement/cercador/BancConeixement/new_space_a_catalunya) a Catalunya.

# Copyright
This code has been developed by Fundació Privada Internet i Innovació Digital a Catalunya (i2CAT). i2CAT is a *non-profit research and innovation centre* that  promotes mission-driven knowledge to solve business challenges, co-create solutions with a transformative impact, empower citizens through open and participative digital social innovation with territorial capillarity, and promote pioneering and strategic initiatives. i2CAT *aims to transfer* research project results to private companies in order to create social and economic impact via the out-licensing of intellectual property and the creation of spin-offs.
Find more information of i2CAT projects and IP rights at https://i2cat.net/tech-transfer/

# Licence -- To do by KTM (AGPL/Comercial)
This code is licensed under the GNU AFFERO GENERAL PUBLIC LICENSE. Information about the license can be found at (https://www.gnu.org/licenses/agpl-3.0.en.html).

If you find that this license doesn't fit with your requirements regarding the use, distribution or redistribution of our code for your specific work, please, don’t hesitate to contact the intellectual property managers in i2CAT at the following address: techtransfer@i2cat.net.