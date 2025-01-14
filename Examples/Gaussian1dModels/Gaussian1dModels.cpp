/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/quantlib.hpp>
#include <boost/timer.hpp>

using namespace QuantLib;

#ifdef BOOST_MSVC
#  ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#    include <ql/auto_link.hpp>
#    define BOOST_LIB_NAME boost_system
#    include <boost/config/auto_link.hpp>
#    undef BOOST_LIB_NAME
#    define BOOST_LIB_NAME boost_thread
#    include <boost/config/auto_link.hpp>
#    undef BOOST_LIB_NAME
#  endif
#endif


#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

Integer sessionId() { return 0; }
}
#endif

// helper function that prints a basket of calibrating swaptions to std::cout

void printBasket(
    const std::vector<boost::shared_ptr<CalibrationHelper> > &basket) {
    std::cout << "\n" << std::left << std::setw(20) << "Expiry" << std::setw(20)
              << "Maturity" << std::setw(20) << "Nominal" << std::setw(14)
              << "Rate" << std::setw(12) << "Pay/Rec" << std::setw(14)
              << "Market ivol" << std::fixed << std::setprecision(6)
              << std::endl;
    std::cout << "===================="
                 "===================="
                 "===================="
                 "===================="
                 "==================" << std::endl;
    for (Size j = 0; j < basket.size(); ++j) {
        boost::shared_ptr<SwaptionHelper> helper =
            boost::dynamic_pointer_cast<SwaptionHelper>(basket[j]);
        Date endDate = helper->underlyingSwap()->fixedSchedule().dates().back();
        Real nominal = helper->underlyingSwap()->nominal();
        Real vol = helper->volatility()->value();
        Real rate = helper->underlyingSwap()->fixedRate();
        Date expiry = helper->swaption()->exercise()->date(0);
        VanillaSwap::Type type = helper->swaption()->type();
        std::ostringstream expiryString, endDateString;
        expiryString << expiry;
        endDateString << endDate;
        std::cout << std::setw(20) << expiryString.str() << std::setw(20)
                  << endDateString.str() << std::setw(20) << nominal
                  << std::setw(14) << rate << std::setw(12)
                  << (type == VanillaSwap::Payer ? "Payer" : "Receiver")
                  << std::setw(14) << vol << std::endl;
    }
}

// helper function that prints the result of a model calibraiton to std::cout

void printModelCalibration(
    const std::vector<boost::shared_ptr<CalibrationHelper> > &basket,
    const Array &volatility) {

    std::cout << "\n" << std::left << std::setw(20) << "Expiry" << std::setw(14)
              << "Model sigma" << std::setw(20) << "Model price"
              << std::setw(20) << "market price" << std::setw(14)
              << "Model ivol" << std::setw(14) << "Market ivol" << std::fixed
              << std::setprecision(6) << std::endl;
    std::cout << "===================="
                 "===================="
                 "===================="
                 "===================="
                 "====================" << std::endl;

    for (Size j = 0; j < basket.size(); ++j) {
        boost::shared_ptr<SwaptionHelper> helper =
            boost::dynamic_pointer_cast<SwaptionHelper>(basket[j]);
        Date expiry = helper->swaption()->exercise()->date(0);
        std::ostringstream expiryString;
        expiryString << expiry;
        std::cout << std::setw(20) << expiryString.str() << std::setw(14)
                  << volatility[j] << std::setw(20) << basket[j]->modelValue()
                  << std::setw(20) << basket[j]->marketValue() << std::setw(14)
                  << basket[j]->impliedVolatility(basket[j]->modelValue(), 1E-6,
                                                  1000, 0.0, 2.0)
                  << std::setw(14) << basket[j]->volatility()->value()
                  << std::endl;
    }
    if (volatility.size() > basket.size()) // only for markov model
        std::cout << std::setw(20) << " " << volatility.back() << std::endl;
}

void printModelAdjuster(
    const std::vector<boost::shared_ptr<CalibrationHelperBase> > &basket,
    const Array &adjuster) {

    std::cout << "\n" << std::left << std::setw(20) << "Expiry" << std::setw(14)
              << "Adjuster" << std::setw(20) << "Model price" << std::setw(20)
              << "Reference price" << std::endl
              << std::setprecision(4);
    std::cout << "===================="
                 "===================="
                 "===================="
                 "====================" << std::endl;

    for (Size j = 0; j < basket.size(); ++j) {
        boost::shared_ptr<AdjusterHelper> helper =
            boost::dynamic_pointer_cast<AdjusterHelper>(basket[j]);
        Date expiry = helper->fixingDate();
        std::ostringstream expiryString;
        expiryString << expiry;
        std::cout << std::setw(20) << expiryString.str() << std::setw(14)
                  << adjuster[j] << std::setw(20) << helper->modelValue()
                  << std::setw(20) << helper->referenceValue() << std::endl;
    }
}

// helper function that prints timing information to std::cout

class Timer {
    boost::timer timer_;
    double elapsed_;

  public:
    void start() { timer_ = boost::timer(); }
    void stop() { elapsed_ = timer_.elapsed(); }
    double elapsed() const { return elapsed_; }
};

void printTiming(const Timer &timer) {
    double seconds = timer.elapsed();
    std::cout << std::fixed << std::setprecision(1) << "\n(this step took "
              << seconds << "s)" << std::endl;
}

// here the main part of the code starts

int main(int argc, char *argv[]) {

    try {

        std::cout << "\nGaussian1dModel Examples" << std::endl;

        std::cout << "\nThis is some example code showing how to use the GSR "
                     "\n(Gaussian short rate) and Markov Functional model."
                  << std::endl;

        Timer timer;

        Date refDate(30, April, 2014);
        Settings::instance().evaluationDate() = refDate;

        std::cout << "\nThe evaluation date for this example is set to "
                  << Settings::instance().evaluationDate() << std::endl;

        Real forward6mLevel = 0.025;
        Real oisLevel = 0.02;

        Handle<Quote> forward6mQuote(
            boost::make_shared<SimpleQuote>(forward6mLevel));
        Handle<Quote> oisQuote(boost::make_shared<SimpleQuote>(oisLevel));

        Handle<YieldTermStructure> yts6m(boost::make_shared<FlatForward>(
            0, TARGET(), forward6mQuote, Actual365Fixed()));
        Handle<YieldTermStructure> ytsOis(boost::make_shared<FlatForward>(
            0, TARGET(), oisQuote, Actual365Fixed()));

        boost::shared_ptr<IborIndex> euribor6m =
            boost::make_shared<Euribor>(6 * Months, yts6m);

        std::cout
            << "\nWe assume a multicurve setup, for simplicity with flat yield "
               "\nterm structures. The discounting curve is an Eonia curve at"
               "\na level of " << oisLevel
            << " and the forwarding curve is an Euribior 6m curve"
            << "\nat a level of " << forward6mLevel << std::endl;

        Real volLevel = 0.20;
        Handle<Quote> volQuote(boost::make_shared<SimpleQuote>(volLevel));
        Handle<SwaptionVolatilityStructure> swaptionVol(
            boost::make_shared<ConstantSwaptionVolatility>(
                0, TARGET(), ModifiedFollowing, volQuote, Actual365Fixed()));

        std::cout
            << "\nFor the volatility we assume a flat swaption volatility at "
            << volLevel << std::endl;

        Real strike = 0.04;
        std::cout << "\nWe consider a standard 10y bermudan payer swaption "
                     "\nwith yearly exercises at a strike of " << strike
                  << std::endl;

        Date effectiveDate = TARGET().advance(refDate, 2 * Days);
        Date maturityDate = TARGET().advance(effectiveDate, 10 * Years);

        Schedule fixedSchedule(effectiveDate, maturityDate, 1 * Years, TARGET(),
                               ModifiedFollowing, ModifiedFollowing,
                               DateGeneration::Forward, false);
        Schedule floatingSchedule(effectiveDate, maturityDate, 6 * Months,
                                  TARGET(), ModifiedFollowing,
                                  ModifiedFollowing, DateGeneration::Forward,
                                  false);

        boost::shared_ptr<NonstandardSwap> underlying =
            boost::make_shared<NonstandardSwap>(VanillaSwap(
                VanillaSwap::Payer, 1.0, fixedSchedule, strike, Thirty360(),
                floatingSchedule, euribor6m, 0.00, Actual360()));

        std::vector<Date> exerciseDates;
        for (Size i = 1; i < 10; ++i)
            exerciseDates.push_back(
                TARGET().advance(fixedSchedule[i], -2 * Days));

        boost::shared_ptr<Exercise> exercise =
            boost::make_shared<BermudanExercise>(exerciseDates, false);
        boost::shared_ptr<NonstandardSwaption> swaption =
            boost::make_shared<NonstandardSwaption>(underlying, exercise);

        std::cout
            << "\nThe model is a one factor Hull White model with piecewise "
               "\nvolatility adapted to our exercise dates." << std::endl;

        std::vector<Date> stepDates(exerciseDates.begin(),
                                    exerciseDates.end() - 1);
        std::vector<Real> sigmas(stepDates.size() + 1, 0.01);
        std::vector<Real> adjusters(stepDates.size() + 1, 1.0);
        Real reversion = 0.01;

        std::cout << "\nThe reversion is just kept constant at a level of "
                  << reversion << std::endl;

        std::cout
            << "\nThe model's curve is set to the 6m forward curve. Note that "
               "\nthe model adapts automatically to other curves where "
               "appropriate "
               "\n(e.g. if an index requires a different forwarding curve) or "
               "\nwhere explicitly specified (e.g. in a swaption pricing "
               "engine)." << std::endl;

        boost::shared_ptr<Gsr> gsr = boost::make_shared<Gsr>(
            yts6m, stepDates, sigmas, reversion, 60.0, adjusters);

        boost::shared_ptr<PricingEngine> swaptionEngine =
            boost::make_shared<Gaussian1dSwaptionEngine>(gsr, 64, 7.0, true,
                                                         false, ytsOis);
        boost::shared_ptr<PricingEngine> nonstandardSwaptionEngine =
            boost::make_shared<Gaussian1dNonstandardSwaptionEngine>(
                gsr, 64, 7.0, true, false, Handle<Quote>(), ytsOis);

        swaption->setPricingEngine(nonstandardSwaptionEngine);

        std::cout
            << "\nThe engine can generate a calibration basket in two modes."
               "\nThe first one is called Naive and generates ATM swaptions "
               "adapted to"
               "\nthe exercise dates of the swaption and its maturity date"
            << std::endl;

        std::cout << "\nThe resulting basket looks as follows:" << std::endl;

        boost::shared_ptr<SwapIndex> swapBase =
            boost::make_shared<EuriborSwapIsdaFixA>(10 * Years, yts6m, ytsOis);

        timer.start();
        std::vector<boost::shared_ptr<CalibrationHelper> > basket =
            swaption->calibrationBasket(swapBase, *swaptionVol,
                                        BasketGeneratingEngine::Naive);
        timer.stop();

        printBasket(basket);
        printTiming(timer);

        std::cout
            << "\nLet's calibrate our model to this basket. We use a "
               "specialized"
               "\ncalibration method calibrating the sigma function one by one "
               "to"
               "\nthe calibrating vanilla swaptions. The result of this is as "
               "follows:" << std::endl;

        for (Size i = 0; i < basket.size(); ++i)
            basket[i]->setPricingEngine(swaptionEngine);

        LevenbergMarquardt method;
        EndCriteria ec(1000, 10, 1E-8, 1E-8,
                       1E-8); // only max iterations use actually used by LM

        timer.start();
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        timer.stop();

        printModelCalibration(basket, gsr->volatility());
        printTiming(timer);

        std::cout << "\nFinally we price our bermudan swaption in the "
                     "calibrated model:" << std::endl;

        timer.start();
        Real npv = swaption->NPV();
        timer.stop();

        std::cout << "\nBermudan swaption NPV (ATM calibrated GSR) = "
                  << std::fixed << std::setprecision(6) << npv << std::endl;
        printTiming(timer);

        std::cout
            << "\nThere is another mode to generate a calibration basket called"
               "\nMaturityStrikeByDeltaGamma. This means that the maturity, "
               "\nthe strike and the nominal of the calibrating swaption are "
               "\ncomputed such that the npv and its first and second "
               "\nderivative with respect to the model's state variable) of"
               "\nthe exotics underlying match with the calibrating swaption's"
               "\nunderlying. Let's try this in our case." << std::endl;

        timer.start();
        basket = swaption->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        timer.stop();

        printBasket(basket);
        printTiming(timer);

        std::cout
            << "\nThe calibrated nominal is close to the exotics nominal."
               "\nThe expiries and maturity dates of the vanillas are the same"
               "\nas in the case above. The difference is the strike which"
               "\nis now equal to the exotics strike." << std::endl;

        std::cout << "\nLet's see how this affects the exotics npv. The "
                     "\nrecalibrated model is:" << std::endl;

        for (Size i = 0; i < basket.size(); ++i)
            basket[i]->setPricingEngine(swaptionEngine);

        timer.start();
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        timer.stop();

        printModelCalibration(basket, gsr->volatility());
        printTiming(timer);

        std::cout << "\nAnd the bermudan's price becomes:" << std::endl;

        timer.start();
        npv = swaption->NPV();
        timer.stop();

        std::cout << "\nBermudan swaption NPV (deal strike calibrated GSR) = "
                  << std::setprecision(6) << npv << std::endl;

        printTiming(timer);

        std::cout
            << "\nWe can do more complicated things, let's e.g. modify the"
               "\nnominal schedule to be linear amortizing and see what"
               "\nthe effect on the generated calibration basket is:"
            << std::endl;

        std::vector<Real> nominalFixed, nominalFloating;
        for (Size i = 0; i < fixedSchedule.size() - 1; ++i) {
            Real tmpNom = 1.0 - (Real)i / (fixedSchedule.size() - 1);
            nominalFixed.push_back(tmpNom);
            nominalFloating.push_back(tmpNom);
            nominalFloating.push_back(
                tmpNom); // we use that the swap is 6m vs. 1y here
        }
        std::vector<Real> strikes(nominalFixed.size(), strike);

        boost::shared_ptr<NonstandardSwap> underlying2(new NonstandardSwap(
            VanillaSwap::Payer, nominalFixed, nominalFloating, fixedSchedule,
            strikes, Thirty360(), floatingSchedule, euribor6m, 1.0, 0.0,
            Actual360()));
        boost::shared_ptr<NonstandardSwaption> swaption2 =
            boost::make_shared<NonstandardSwaption>(underlying2, exercise);

        swaption2->setPricingEngine(nonstandardSwaptionEngine);

        timer.start();
        basket = swaption2->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        timer.stop();

        printBasket(basket);
        printTiming(timer);

        std::cout << "\nThe notional is weighted over the underlying exercised "
                     "\ninto and the maturity is adjusted downwards. The rate"
                     "\non the other hand is not affected." << std::endl;

        std::cout
            << "\nYou can also price exotic bond's features. If you have e.g. a"
               "\nbermudan callable fixed bond you can set up the call right "
               "\nas a swaption to enter into a one leg swap with notional"
               "\nreimbursement at maturity."
               "\nThe exercise should then be written as a rebated exercise"
               "\npaying the notional in case of exercise." << std::endl;

        std::cout << "\nThe calibration basket looks like this:" << std::endl;

        std::vector<Real> nominalFixed2(nominalFixed.size(), 1.0);
        std::vector<Real> nominalFloating2(nominalFloating.size(),
                                           0.0); // null the second leg

        boost::shared_ptr<NonstandardSwap> underlying3(new NonstandardSwap(
            VanillaSwap::Receiver, nominalFixed2, nominalFloating2,
            fixedSchedule, strikes, Thirty360(), floatingSchedule, euribor6m,
            1.0, 0.0, Actual360(), false,
            true)); // final capital exchange

        boost::shared_ptr<RebatedExercise> exercise2 =
            boost::make_shared<RebatedExercise>(*exercise, -1.0, 2, TARGET());

        boost::shared_ptr<NonstandardSwaption> swaption3 =
            boost::make_shared<NonstandardSwaption>(underlying3, exercise2);

        boost::shared_ptr<SimpleQuote> oas0 =
            boost::make_shared<SimpleQuote>(0.0);
        boost::shared_ptr<SimpleQuote> oas100 =
            boost::make_shared<SimpleQuote>(0.01);
        RelinkableHandle<Quote> oas(oas0);

        boost::shared_ptr<PricingEngine> nonstandardSwaptionEngine2 =
            boost::make_shared<Gaussian1dNonstandardSwaptionEngine>(
                gsr, 64, 7.0, true, false, oas); // change discounting to 6m

        swaption3->setPricingEngine(nonstandardSwaptionEngine2);

        timer.start();

        basket = swaption3->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        timer.stop();

        printBasket(basket);
        printTiming(timer);

        std::cout
            << "\nNote that nominals are not exactly 1.0 here. This is"
            << "\nbecause we do our bond discounting on 6m level while"
            << "\nthe swaptions are still discounted on OIS level."
            << "\n(You can try this by changing the OIS level to the "
            << "\n6m level, which will produce nominals near 1.0)."
            << "\nThe npv of the call right is (after recalibrating the model)"
            << std::endl;

        for (Size i = 0; i < basket.size(); i++)
            basket[i]->setPricingEngine(swaptionEngine);

        timer.start();
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        Real npv3 = swaption3->NPV();
        timer.stop();

        std::cout << "\nBond's bermudan call right npv = "
                  << std::setprecision(6) << npv3 << std::endl;
        printTiming(timer);

        std::cout
            << "\nUp to now, no credit spread is included in the pricing."
               "\nWe can do so by specifying an oas in the pricing engine."
               "\nLet's set the spread level to 100bp and regenerate"
               "\nthe calibration basket." << std::endl;

        oas.linkTo(oas100);

        timer.start();
        basket = swaption3->calibrationBasket(
            swapBase, *swaptionVol,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
        timer.stop();
        printBasket(basket);
        printTiming(timer);

        std::cout
            << "\nThe adjusted basket takes the credit spread into account."
               "\nThis is consistent to a hedge where you would have a"
               "\nmargin on the float leg around 100bp,too." << std::endl;

        std::cout << "\nThe npv becomes:" << std::endl;

        for (Size i = 0; i < basket.size(); i++)
            basket[i]->setPricingEngine(swaptionEngine);

        timer.start();
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        Real npv4 = swaption3->NPV();
        timer.stop();

        std::cout << "\nBond's bermudan call right npv (oas = 100bp) = "
                  << std::setprecision(6) << npv4 << std::endl;
        printTiming(timer);

        std::cout
            << "\nThe next instrument we look at is a CMS 10Y vs Euribor "
               "\n6M swaption. The maturity is again 10 years and the option"
               "\nis exercisable on a yearly basis" << std::endl;

        boost::shared_ptr<FloatFloatSwap> underlying4(new FloatFloatSwap(
                VanillaSwap::Payer, 1.0, 1.0, fixedSchedule, swapBase,
                Thirty360(), floatingSchedule, euribor6m, Actual360(), false,
                false, 1.0, 0.0, Null<Real>(), Null<Real>(), 1.0, 0.0010));

        boost::shared_ptr<FloatFloatSwaption> swaption4 =
            boost::make_shared<FloatFloatSwaption>(underlying4, exercise);

        boost::shared_ptr<Gaussian1dFloatFloatSwaptionEngine>
            floatSwaptionEngine =
                boost::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
                    gsr, 64, 7.0, true, false, Handle<Quote>(), ytsOis, true,
                    Gaussian1dFloatFloatSwaptionEngine::None, true);

        swaption4->setPricingEngine(floatSwaptionEngine);

        std::cout
            << "\nSince the underlying is quite exotic already, we start with"
               "\npricing this using the LinearTsrPricer for CMS coupon "
               "estimation" << std::endl;

        Handle<Quote> reversionQuote(
            boost::make_shared<SimpleQuote>(reversion));

        const Leg &leg0 = underlying4->leg(0);
        const Leg &leg1 = underlying4->leg(1);
        boost::shared_ptr<CmsCouponPricer> cmsPricer =
            boost::make_shared<LinearTsrPricer>(swaptionVol, reversionQuote);
        boost::shared_ptr<IborCouponPricer> iborPricer(new BlackIborCouponPricer);

        setCouponPricer(leg0, cmsPricer);
        setCouponPricer(leg1, iborPricer);

        boost::shared_ptr<PricingEngine> swapPricer =
            boost::make_shared<DiscountingSwapEngine>(ytsOis);

        underlying4->setPricingEngine(swapPricer);

        timer.start();
        Real npv5 = underlying4->NPV();
        timer.stop();

        std::cout << "Underlying CMS Swap NPV = " << std::setprecision(6)
                  << npv5 << std::endl;
        std::cout << "       CMS     Leg  NPV = " << underlying4->legNPV(0)
                  << std::endl;
        std::cout << "       Euribor Leg  NPV = " << underlying4->legNPV(1)
                  << std::endl;

        printTiming(timer);

        std::cout << "\nWe generate a naive calibration basket and calibrate "
                     "\nthe GSR model to it:" << std::endl;

        timer.start();
        basket = swaption4->calibrationBasket(swapBase, *swaptionVol,
                                              BasketGeneratingEngine::Naive);
        for (Size i = 0; i < basket.size(); ++i)
            basket[i]->setPricingEngine(swaptionEngine);
        gsr->calibrateVolatilitiesIterative(basket, method, ec);
        timer.stop();

        printBasket(basket);
        printModelCalibration(basket, gsr->volatility());
        printTiming(timer);

        std::cout << "\nThe npv of the bermudan swaption is" << std::endl;

        timer.start();
        Real npv6 = swaption4->NPV();
        timer.stop();

        std::cout << "\nFloat swaption NPV (GSR) = " << std::setprecision(6)
                  << npv6 << std::endl;
        printTiming(timer);

        std::cout << "\nIn this case it is also interesting to look at the "
                     "\nunderlying swap npv in the GSR model." << std::endl;

        std::cout << "\nFloat swap NPV (GSR) = " << std::setprecision(6)
                  << swaption4->result<Real>("underlyingValue") << std::endl;

        std::cout << "\nNot surprisingly, the underlying is priced differently"
                     "\ncompared to the LinearTsrPricer, since a different"
                     "\nsmile is implied by the GSR model." << std::endl;

        std::cout << "\nThis is exactly where the Markov functional model"
                  << "\ncomes into play, because it can calibrate to any"
                  << "\ngiven underlying smile (as long as it is arbitrage"
                  << "\nfree). We try this now. Of course the usual use case"
                  << "\nis not to calibrate to a flat smile as in our simple"
                  << "\nexample, still it should be possible, of course..."
                  << std::endl;

        std::vector<Date> markovStepDates(exerciseDates.begin(),
                                          exerciseDates.end());
        std::vector<Date> cmsFixingDates(markovStepDates);
        std::vector<Real> markovSigmas(markovStepDates.size() + 1, 0.01);
        std::vector<Period> tenors(cmsFixingDates.size(), 10 * Years);
        boost::shared_ptr<MarkovFunctional> markov =
            boost::make_shared<MarkovFunctional>(
                yts6m, reversion, markovStepDates, markovSigmas, swaptionVol,
                cmsFixingDates, tenors, swapBase,
                MarkovFunctional::ModelSettings().withYGridPoints(16));

        boost::shared_ptr<Gaussian1dSwaptionEngine> swaptionEngineMarkov =
            boost::make_shared<Gaussian1dSwaptionEngine>(markov, 8, 5.0, true,
                                                         false, ytsOis);
        boost::shared_ptr<Gaussian1dFloatFloatSwaptionEngine>
            floatEngineMarkov =
                boost::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
                    markov, 16, 7.0, true, false, Handle<Quote>(), ytsOis,
                    true);

        swaption4->setPricingEngine(floatEngineMarkov);

        timer.start();
        Real npv7 = swaption4->NPV();
        timer.stop();

        std::cout << "\nThe option npv is the markov model is:" << std::endl;

        std::cout << "\nFloat swaption NPV (Markov) = " << std::setprecision(6)
                  << npv7 << std::endl;
        printTiming(timer);

        std::cout << "\nThis is not too far from the GSR price." << std::endl;

        std::cout << "\nMore interesting is the question how well the Markov"
                  << "\nmodel did its job to match our input smile. For this"
                  << "\nwe look at the underlying npv under the Markov model"
                  << std::endl;

        std::cout << "\nFloat swap NPV (Markov) = " << std::setprecision(6)
                  << swaption4->result<Real>("underlyingValue") << std::endl;

        std::cout << "\nThis is closer to our terminal swap rate model price."
                     "\nA perfect match is not expected anyway, because the"
                     "\ndynamics of the underlying rate in the linear"
                     "\nmodel is different from the Markov model, of"
                     "\ncourse." << std::endl;

        std::cout << "\nThe Markov model can not only calibrate to the"
                     "\nunderlying smile, but has at the same time a"
                     "\nsigma function (similar to the GSR model) which"
                     "\ncan be used to calibrate to a second instrument"
                     "\nset. We do this here to calibrate to our coterminal"
                     "\nATM swaptions from above." << std::endl;

        std::cout << "\nThis is a computationally demanding task, so"
                     "\ndepending on your machine, this may take a"
                     "\nwhile now..." << std::endl;

        for (Size i = 0; i < basket.size(); ++i)
            basket[i]->setPricingEngine(swaptionEngineMarkov);

        timer.start();
        markov->calibrate(basket, method, ec);
        timer.stop();

        printModelCalibration(basket, markov->volatility());
        printTiming(timer);

        std::cout << "\nNow let's have a look again at the underlying pricing."
                     "\nIt shouldn't have changed much, because the underlying"
                     "\nsmile is still matched." << std::endl;

        timer.start();
        Real npv8 = swaption4->result<Real>("underlyingValue");
        timer.stop();
        std::cout << "\nFloat swap NPV (Markov) = " << std::setprecision(6)
                  << npv8 << std::endl;
        printTiming(timer);

        std::cout << "\nThis is close to the previous value as expected."
                  << std::endl;

        std::cout << "\nAs a final remark we note that the calibration to"
                  << "\ncoterminal swaptions is not particularly reasonable"
                  << "\nhere, because the european call rights are not"
                  << "\nwell represented by these swaptions."
                  << "\nSecondly, our CMS swaption is sensitive to the"
                  << "\ncorrelation between the 10y swap rate and the"
                  << "\nEuribor 6M rate. Since the Markov model is one factor"
                  << "\nit will most probably underestimate the market value"
                  << "\nby construction." << std::endl;

        std::cout << "\nThere is a way to enforce the underlying match"
                  << "\nwe saw in the Markov model also in the Gsr model"
                  << "\nby so called internal adjusters. These are factors"
                  << "\nfor the model volatility used in case the exotic"
                  << "\ncoupons in question (here the CMS coupons) are"
                  << "\nevaluated. The factors are calibrated such that"
                  << "\na reference market price (here the price from"
                  << "\nthe linear replication model) is matched." << std::endl;

        swaption4->setPricingEngine(floatSwaptionEngine);

        std::vector<boost::shared_ptr<CalibrationHelperBase> > adjusterBasket;
        for (Size i = 0; i < leg0.size(); ++i) {
            boost::shared_ptr<CmsCoupon> coupon =
                boost::dynamic_pointer_cast<CmsCoupon>(leg0[i]);
            if (coupon->fixingDate() > refDate) {
                boost::shared_ptr<AdjusterHelper> tmp =
                    boost::make_shared<AdjusterHelper>(
                        swapBase, coupon->fixingDate(), coupon->date());
                tmp->setCouponPricer(cmsPricer);
                tmp->setPricingEngine(floatSwaptionEngine);
                adjusterBasket.push_back(tmp);
            }
        }

        std::cout << "\nWe calibrate adjusters in our setup here:" << std::endl;

        timer.start();
        gsr->calibrateAdjustersIterative(adjusterBasket, method, ec);
        timer.stop();
        printModelAdjuster(adjusterBasket, gsr->adjuster());
        printTiming(timer);

        std::cout << "\nThe resulting option and underlying value"
                  << "\nin the adjusted Gsr model are:" << std::endl;

        Real npv9 = swaption4->NPV();
        Real npv10 = swaption4->result<Real>("underlyingValue");
        std::cout << std::setprecision(6)
                  << "GSR (adjusted) option value = " << npv9 << std::endl;
        std::cout << "GSR (adjusted) underlying value = " << npv10 << std::endl;

        std::cout << "\nThat was it. Thank you for running this demo. Bye."
                  << std::endl;

    } catch (QuantLib::Error e) {
        std::cout << "terminated with a ql exception: " << e.what()
                  << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "terminated with a general exception: " << e.what()
                  << std::endl;
        return 1;
    }
}
