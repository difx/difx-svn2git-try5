
// g++ -Wall -g -O0 autobands_test.cpp freq.cpp autobands.cpp -o autobands

#include "autobands.h"

#include <algorithm>
#include <iostream>

int main(int argc, char** argv)
{
	const double bw_hz = 500e6; //58e6;
	const double fftSpecRes = 15.625e3;
	const double outSpecRes = 0.5e6;
	AutoBands a(bw_hz, 4, false);

	std::vector<double> recstart;
	std::vector<double> recstop;
	std::vector<double> ftest;

	/// Test the GCD method

	ftest.push_back(1.0);
	ftest.push_back(0.75);
	ftest.push_back(1.5);
	ftest.push_back(1.5);
	a.getGranularity(ftest); // expected: 0.25

	//// EHT VEX

	// 4 x standard EHT 230G
	recstart.clear();
	recstop.clear();
	recstart.push_back(212052000000.0);
	recstop.push_back(214100000000.0);
	a.addRecbands(recstart, recstop);
	a.addRecbands(recstart, recstop);
	a.addRecbands(recstart, recstop);
	a.addRecbands(recstart, recstop);

	// 1 x offset EHT 230G
	recstart.clear();
	recstop.clear();
	recstart.push_back(212055000000.0);
	recstop.push_back(214103000000.0);
	a.addRecbands(recstart, recstop);

	// 1 x ALMA, LSB ordering of USB channels
	recstart.clear();
	recstop.clear();
	for (double n = 0; n < 32; n++)
	{
		recstart.push_back(213976953125.0 - n*58593750.0);
		recstop.push_back(214039453125.0 - n*58593750.0);
	}
	a.addRecbands(recstart, recstop);

	/// Auto band magic

	double suggested_bw = a.autoBandwidth();
	std::cout << "Autobands::autoBandwidth() suggests " << suggested_bw*1e-6 << " MHz, ignoring it though!\n";
	a.generateOutputbands();

	// Summary
	std::cout << a;

	// Frequency and destination band lookup
	std::vector<freq> allfreqs;
	for(unsigned n = 0; n < a.outputbands.size(); n++)
	{
		const AutoBands::Outputband& ob = a.outputbands[n];
		freq fq(ob.fbandstart, ob.bandwidth, 'U', /*unused but needed:*/345.0);
		allfreqs.push_back(fq);

		for(unsigned m = 0; m < ob.constituents.size(); m++)
		{
			const AutoBands::Band& band = ob.constituents[m];
			freq fq(band.flow, band.fhigh - band.flow, 'U', /*unused but needed:*/123.0);
			if (std::find(allfreqs.begin(), allfreqs.end(), fq) == allfreqs.end())
			{
				allfreqs.push_back(fq);
			}
		}
	}

	std::cout << "Allfreqs: ";
	for (unsigned n = 0; n < allfreqs.size(); n++)
	{
		std::cout << allfreqs[n].fq*1e-6 << "/" << allfreqs[n].bw*1e-6 << " ";
	}
	std::cout << "\n";

	int imid = a.outputbands.size() / 2;
	int cmid = a.outputbands[imid].constituents.size() - 1;
	AutoBands::Band& band = a.outputbands[imid][cmid];
	std::cout << "\nLooking up outputband #" << imid << " constituent band #" << cmid << " : " << band << "\n";
	freq fq(band.flow, band.fhigh - band.flow, 'U', /*unused but needed:*/456.0);
	int idx = a.lookupDestinationFreq(fq, allfreqs);
	std::cout << "Result : fq #" << idx << "\n";

	std::cout << "\nChecking completeness:\n";
	for (int n=0; n<a.outputbands.size(); n++) {
		std::cout << "\n"
			<< "   Outputband[" << n << "] = " << a.outputbands[n]
			<< "      complete=" << a.outputbands[n].isComplete() << "\n";
	}

	std::cout << "\nListing the intra-band 'edge' channels that need flagging:\n";
	for (int n=0; n<a.outputbands.size(); n++) {
		std::deque<int> channels;
		a.listEdgeChannels(a.outputbands[n], channels, fftSpecRes, outSpecRes);
	}

    float limit = 0.25;
	std::cout << "\nListing the intra-band 'edge' channels, now allowing max " << (int)(limit*100) << "% of band mixing:\n";
	for (int n=0; n<a.outputbands.size(); n++) {
		std::deque<int> channels;
		a.listEdgeChannels(a.outputbands[n], channels, fftSpecRes, outSpecRes, limit);
	}

	return 0;
}
