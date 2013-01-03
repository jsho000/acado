/*
 *    This file is part of ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2009 by Boris Houska and Hans Joachim Ferreau, K.U.Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC) under
 *    supervision of Moritz Diehl. All rights reserved.
 *
 *    ACADO Toolkit is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    ACADO Toolkit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with ACADO Toolkit; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */



/**
 *    \file src/code_generation/integrator_export.cpp
 *    \author Hans Joachim Ferreau, Boris Houska, Rien Quirynen
 *    \date 2010-2011
 */

#include <acado/code_generation/integrators/integrator_export.hpp>



BEGIN_NAMESPACE_ACADO


//
// PUBLIC MEMBER FUNCTIONS:
//

IntegratorExport::IntegratorExport(	UserInteraction* _userInteraction,
									const String& _commonHeaderName
									) : ExportAlgorithm( _userInteraction,_commonHeaderName )
{
	EXPORT_RHS = BT_TRUE;
	EQUIDISTANT = BT_TRUE;
	CRS_FORMAT = BT_FALSE;

	reset_int = ExportVariable( "resetIntegrator", 1, 1, INT, ACADO_VARIABLES, BT_TRUE );
}


IntegratorExport::IntegratorExport(	const IntegratorExport& arg
									) : ExportAlgorithm( arg )
{
	EXPORT_RHS = BT_TRUE;
	EQUIDISTANT = BT_TRUE;
	CRS_FORMAT = BT_FALSE;
}


IntegratorExport::~IntegratorExport( )
{
	clear( );
}


IntegratorExport& IntegratorExport::operator=(	const IntegratorExport& arg
												)
{
	if( this != &arg && &arg != 0 )
	{
		clear( );
		ExportAlgorithm::operator=( arg );
	}
    return *this;
}


returnValue IntegratorExport::setGrid(	const Grid& _grid )
{
	grid = _grid;
	EQUIDISTANT = BT_FALSE;

	return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::setGrid(	const Grid& _ocpGrid, const uint _numSteps
										)
{
	uint i;
	N = _ocpGrid.getNumIntervals();
	BooleanType equidistant = _ocpGrid.isEquidistant();
	double T = _ocpGrid.getLastTime() - _ocpGrid.getFirstTime();
	double h = T/((double)_numSteps);
	Vector stepsVector( N );

	for( i = 0; i < stepsVector.getDim(); i++ )
	{
		stepsVector(i) = (int) ceil((_ocpGrid.getTime(i+1)-_ocpGrid.getTime(i))/h - 10.0*EPS);
	}

	if( equidistant )
	{
		// Setup fixed integrator grid for equidistant control grid
		grid = Grid( 0.0, ((double) T)/((double) N), (int) ceil((double)_numSteps/((double) N) - 10.0*EPS) + 1 );
	}
	else
	{
		// Setup for non equidistant control grid
		// NOTE: This grid defines only one integration step because the control
		// grid is non equidistant.
		grid = Grid( 0.0, h, 2 );
		numSteps = stepsVector;
	}
	return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::setModel(	const String& _name_ODE, const String& _name_diffs_ODE ) {

	if( ODE.getFunctionDim() == 0 ) {
		name_ODE = String(_name_ODE);
		name_diffs_ODE = String(_name_diffs_ODE);

		EXPORT_RHS = BT_FALSE;
	}
	else {
		return ACADOERROR( RET_INVALID_OPTION );
	}

	return SUCCESSFUL_RETURN;
}



// PROTECTED:


returnValue IntegratorExport::copy(	const IntegratorExport& arg
									)
{
	EXPORT_RHS = arg.EXPORT_RHS;
	EQUIDISTANT = arg.EQUIDISTANT;
	CRS_FORMAT = arg.CRS_FORMAT;
	grid = arg.grid;
	numSteps = arg.numSteps;

	// ExportFunctions
	integrate = arg.integrate;
	
	return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::clear( )
{
	return SUCCESSFUL_RETURN;
}


uint IntegratorExport::getIntegrationInterval( double time ) {
	uint index = 0;
	double scale = 1.0/(grid.getLastTime() - grid.getFirstTime());
	while( index < (grid.getNumIntervals()-1) && time > scale*grid.getTime( index+1 ) ) {
		index++;
	}
	return index;
}


returnValue IntegratorExport::getGrid( Grid& grid_ ) const{

    grid_ = grid;
    return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::getNumSteps( Vector& _numSteps ) const{

    _numSteps = numSteps;
    return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::getOutputExpressions( std::vector<Expression>& outputExpressions_ ) const{

    outputExpressions_ = outputExpressions;
    return SUCCESSFUL_RETURN;
}


returnValue IntegratorExport::getOutputGrids( std::vector<Grid>& outputGrids_ ) const{

    outputGrids_ = outputGrids;
    return SUCCESSFUL_RETURN;
}

BooleanType IntegratorExport::equidistantControlGrid( ) const{
	
	return numSteps.isEmpty();
}

const String IntegratorExport::getNameODE() const{
	if( EXPORT_RHS ) {
		return ODE.getName();
	}
	else {
		return name_ODE;
	}
}

const String IntegratorExport::getNameOUTPUT( uint index ) const{
	if( EXPORT_RHS ) {
		return OUTPUTS[index].getName();
	}
	else {
		return name_OUTPUTS[index];
	}
}

uint IntegratorExport::getDimOUTPUT( uint index ) const{
	if( EXPORT_RHS ) {
		return outputExpressions[index].getDim();
	}
	else {
		return num_OUTPUTS[index];
	}
}


const String IntegratorExport::getNameDiffsODE() const{
	if( EXPORT_RHS ) {
		return diffs_ODE.getName();
	}
	else {
		return name_diffs_ODE;
	}
}

const String IntegratorExport::getNameDiffsOUTPUT( uint index ) const{
	if( EXPORT_RHS ) {
		return diffs_OUTPUTS[index].getName();
	}
	else {
		return name_diffs_OUTPUTS[index];
	}
}


CLOSE_NAMESPACE_ACADO

// end of file.
