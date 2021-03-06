/***************************************************************************
 *   Copyright (C) 2015 by Walter Brisken                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*===========================================================================
 * SVN properties (DO NOT CHANGE)
 *
 * $Id$
 * $HeadURL: https://svn.atnf.csiro.au/difx/applications/vex2difx/trunk/src/util.h $
 * $LastChangedRevision$
 * $Author$
 * $LastChangedDate$
 *
 *==========================================================================*/

#include <cstdlib>
#include <sstream>
#include <map>
#include "makejobs.h"
#include "mediachange.h"

// A is assumed to be the first scan in time order
bool areScansCompatible(const VexScan *A, const VexScan *B, const CorrParams *P)
{
	if(((B->mjdStart < A->mjdStop) && (fabs(B->mjdStart-A->mjdStop) > 1.0e-8)) ||
	   (B->mjdStart > A->mjdStop + P->maxGap))
	{
		return false;
	}
	if(A->overlap(*B) >= -0.1/86400.0)
	{
		return false;
	}
	if(P->singleScan)
	{
		return false;
	}
	if(P->singleSetup && A->modeDefName != B->modeDefName)
	{
		return false;
	}
	
	return true;
}


// Divides scans into different groups where each group contains scans that can be correlated at the same time.
// This does not pay attention to media or clock breaks
static void genJobGroups(std::vector<VexJobGroup> &JGs, const VexData *V, const CorrParams *P, int verbose)
{
	unsigned int nNoRecordScan = 0;
	std::list<std::string> scans;
	V->getScanList(scans);

	while(!scans.empty())
	{
		const VexScan *scan = V->getScanByDefName(scans.front());
		unsigned int nRecordedAnt = scan->nAntennasWithRecordedData(V);

		if(nRecordedAnt < P->minSubarraySize)
		{
			if(verbose > 2)
			{
				std::cout << "Skipping scan " << scans.front() << " because it has no recorded data." << std::endl;
			}
			
			scans.pop_front();
			++nNoRecordScan;

			continue;
		}
		JGs.push_back(VexJobGroup());
		VexJobGroup &JG = JGs.back();
		JG.scans.push_back(scans.front());
		JG.setTimeRange(*scan);
		scans.pop_front();

		const VexScan *scan1 = V->getScanByDefName(JG.scans.back());
		const CorrSetup *corrSetup1 = P->getCorrSetup(scan1->corrSetupName);

		for(std::list<std::string>::iterator it = scans.begin(); it != scans.end();)
		{
			const VexScan *scan2 = V->getScanByDefName(*it);
			const CorrSetup *corrSetup2 = P->getCorrSetup(scan2->corrSetupName);

			// Skip any scans that don't overlap with .v2d mjdStart and mjdStop
			if(P->overlap(*scan2) <= 0.0)
			{
				++it;
				continue;
			}

#warning "FIXME: verify modes are compatible"
			if(areCorrSetupsCompatible(corrSetup1, corrSetup2, P) &&
			   areScansCompatible(scan1, scan2, P))
			{
				JG.logicalOr(*scan2);	// expand jobGroup time to include this scan
				JG.scans.push_back(*it);
				it = scans.erase(it);
				scan1 = scan2;
				corrSetup1 = corrSetup2;
			}
			else
			{	
				++it;
			}
		}
	}

	if(verbose + nNoRecordScan > 0)
	{
		std::cout << nNoRecordScan << " scans dropped because they recorded no baseband data." << std::endl;
	}

	const std::list<VexEvent> *events = V->getEvents();
	for(std::vector<VexJobGroup>::iterator jg = JGs.begin(); jg != JGs.end(); ++jg)
	{
		jg->genEvents(*events);
		jg->logicalAnd(*P);		// possibly shrink job group to requested range
	}
}

static void genJobs(std::vector<VexJob> &Js, const VexJobGroup &JG, VexData *V, const CorrParams *P, int verbose)
{
	std::map<std::string,double> recordStop;
	std::map<double,int> usage;
	std::map<double,int> clockBreaks;
	int nClockBreaks = 0;
	std::list<MediaChange> changes;
	std::list<double> times;
	std::list<double> breaks;
	double mjdLast = -1.0;
	double mjdBest = 0.0;
	double start;
	int nAnt;
	int nLoop = 0;
	Interval scanRange;

	// first initialize recordStop and usage
	for(std::list<VexEvent>::const_iterator e = JG.events.begin(); e != JG.events.end(); ++e)
	{
		if(e->eventType == VexEvent::RECORD_START)
		{
			recordStop[e->name] = -1.0;
		}
		if(e->eventType == VexEvent::SCAN_START && (scanRange.mjdStart < 1.0 || e->mjd < scanRange.mjdStart))
		{
			scanRange.mjdStart = e->mjd;
		}
		if(e->eventType == VexEvent::SCAN_START && (scanRange.mjdStart < 1.0 || e->mjd > scanRange.mjdStop))
		{
			scanRange.mjdStop = e->mjd;
		}

		usage[e->mjd] = 0;
		clockBreaks[e->mjd] = 0;
	}
	nAnt = recordStop.size();

	scanRange.logicalAnd(*P);	// Shrink time range to v2d start / stop interval

	// populate changes, times, and usage
	for(std::list<VexEvent>::const_iterator e = JG.events.begin(); e != JG.events.end(); ++e)
	{
		if(mjdLast > 0.0 && e->mjd > mjdLast)
		{
			usage[e->mjd] = usage[mjdLast];
			mjdLast = e->mjd;
			if(JG.containsAbsolutely(e->mjd))
			{
				times.push_back(e->mjd);
			}
		}
		else if(mjdLast < 0.0)
		{
			usage[e->mjd] = 0;
			mjdLast = e->mjd;
			if(JG.containsAbsolutely(e->mjd))
			{
				times.push_back(e->mjd);
			}
		}

		if(e->eventType == VexEvent::RECORD_START)
		{
			if(recordStop[e->name] > 0.0)
			{
				if(JG.containsAbsolutely(recordStop[e->name]) &&
				   JG.containsAbsolutely(e->mjd) &&
				   scanRange.containsAbsolutely(e->mjd))
				{
					changes.push_back(MediaChange(e->name, recordStop[e->name], e->mjd));
					if(verbose > 0)
					{
						std::cout << "Media change: " << e->name << " " << (Interval)(changes.back()) << std::endl;
					}
				}
			}
		}
		else if(e->eventType == VexEvent::RECORD_STOP)
		{
			recordStop[e->name] = e->mjd;
		}
		else if(e->eventType == VexEvent::ANT_SCAN_START)
		{
			++usage[e->mjd];
		}
		else if(e->eventType == VexEvent::ANT_SCAN_STOP)
		{
			--usage[e->mjd];
		}
		else if(e->eventType == VexEvent::CLOCK_BREAK ||
			e->eventType == VexEvent::LEAP_SECOND ||
			e->eventType == VexEvent::ANTENNA_START ||
			e->eventType == VexEvent::ANTENNA_STOP ||
			e->eventType == VexEvent::MANUAL_BREAK)
		{
			if(JG.containsAbsolutely(e->mjd))
			{
				++clockBreaks[e->mjd];
				++nClockBreaks;
			}
		}
	}

	// now go through and set breakpoints
	while(!changes.empty() || nClockBreaks > 0)
	{
		int scoreBest;
		int nEvent = JG.events.size();

		++nLoop;
		if(nLoop > nEvent+3) // There is clearly a problem converging!
		{
			std::cerr << "Developer error: jobs not converging after " << nLoop << " tries.\n" << std::endl;

			std::cerr << "Events:" << std::endl;
			std::list<VexEvent>::const_iterator iter;
			for(iter = JG.events.begin(); iter != JG.events.end(); ++iter)
			{
				std::cerr << "   " << *iter << std::endl;
			}

			std::cerr << "nClockBreaks = " << nClockBreaks << std::endl;

			std::cerr << "Media Changes remaining were:" << std::endl;
			std::list<MediaChange>::const_iterator it;
			for(it = changes.begin(); it != changes.end(); ++it)
			{
				std::cerr << "   " << *it << std::endl;
			}

			exit(EXIT_FAILURE);
		}

		// look for break with highest score
		// Try as hard as possible to minimize number of breaks
		scoreBest = -1;
		for(std::list<double>::const_iterator t = times.begin(); t != times.end(); ++t)
		{
			int score = nGap(changes, *t) * (nAnt-usage[*t]+1) + 100*clockBreaks[*t];
			if(score > scoreBest)
			{
				scoreBest = score;
				mjdBest = *t;
			}
		}

		breaks.push_back(mjdBest);
		nClockBreaks -= clockBreaks[mjdBest];
		clockBreaks[mjdBest] = 0;

		// find modules that change in the new gap
		for(std::list<MediaChange>::iterator c = changes.begin(); c != changes.end();)
		{
			if(c->mjdStart <= mjdBest && c->mjdStop >= mjdBest)
			{
				c = changes.erase(c);
			}
			else
			{
				++c;
			}
		}
	}
	breaks.sort();

	// Add a break at end so num breaks = num jobs
	breaks.push_back(JG.mjdStop);

	// form jobs
	start = JG.mjdStart;
	for(std::list<double>::const_iterator t = breaks.begin(); t != breaks.end(); ++t)
	{
		Interval jobTimeRange(start, *t);
		if(jobTimeRange.duration() > P->minLength)
		{
			JG.createJobs(Js, jobTimeRange, V, P->maxLength, P->maxSize);
		}
		else
		{
			std::cerr << "Warning: skipping short job of " << (jobTimeRange.duration()*86400.0) << " seconds duration." << std::endl;
		}
		start = *t;
	}
}

void makeJobs(std::vector<VexJob>& J, VexData *V, const CorrParams *P, std::list<std::pair<int,std::string> > &removedAntennas, int verbose)
{
	std::vector<VexJobGroup> JG;

	// Add antenna start/stops

	std::vector<AntennaSetup>::const_iterator as;
	for(as = P->antennaSetups.begin(); as != P->antennaSetups.end(); ++as)
	{
		if(as->mjdStart > 0.0)
		{
			V->addEvent(as->mjdStart, VexEvent::ANTENNA_START, as->vexName);
		}
		if(as->mjdStop > 0.0)
		{
			V->addEvent(as->mjdStop, VexEvent::ANTENNA_STOP, as->vexName);
		}
	}
	V->sortEvents();

	// Do splitting of jobs
	genJobGroups(JG, V, P, verbose);

	if(verbose > 0)
	{
		std::cout << JG.size() << " job groups created:" << std::endl;
		for(std::vector<VexJobGroup>::const_iterator jg = JG.begin(); jg != JG.end(); ++jg)
		{
			std::cout << "  " << *jg;
		}
	}

	for(std::vector<VexJobGroup>::const_iterator jg = JG.begin(); jg != JG.end(); ++jg)
	{
		genJobs(J, *jg, V, P, verbose);
	}

	// Finalize all the new job structures
	int jobId = P->startSeries;
	for(std::vector<VexJob>::iterator j = J.begin(); j != J.end(); ++j)
	{
		std::ostringstream name;
		j->jobSeries = P->jobSeries;
		j->jobId = jobId;

		// note: this is an internal name only, not the job prefix that 
		// becomes part of the filenames
		name << j->jobSeries << "_" << j->jobId;

		V->addEvent(j->mjdStart, VexEvent::JOB_START, name.str());
		V->addEvent(j->mjdStop,  VexEvent::JOB_STOP,  name.str());
		j->assignVSNs(*V);

		// Here we need to check if there really is data for all the stations in the job.
		//   If not, remove the antenna and add to the "no data" table.
		for(std::map<std::string,std::string>::iterator it = j->vsns.begin(); it != j->vsns.end(); ++it)
		{
			if(it->second == "None")
			{
				const AntennaSetup *as = P->getAntennaSetup(it->first);
				if(as->getDataSource() == DataSourceFile)
				{
					if(!as->hasBasebandFile(*j))	// test if all baseband files are out of time range
					{
						removedAntennas.push_back(std::pair<int,std::string>(jobId, it->first));
						if(verbose > 0)
						{
							std::cout << "Removed " << it->first << " from jobId " << jobId << " because no data exists." << std::endl;
						}
						j->vsns.erase(it);

						// this is a bit ugly.  Any better way to remove one item from a map from within?
						it = j->vsns.begin();
						if(it == j->vsns.end())
						{
							break;
						}
					}
				}
			}
		}
		
		// If fewer than minSubarray antennas remain, then mark the job as bad and exclude writing it later.
		if(j->vsns.size() < P->minSubarraySize)
		{
			j->jobSeries = "-";	// Flag to not actually produce this job
		}
		++jobId;
	}
	V->sortEvents();
}
