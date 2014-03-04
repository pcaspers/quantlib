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


  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmsspreadpricer.hpp
    \brief cms spread coupon pricer as in Brigo, ...
*/

#ifndef quantlib_cmsspread_pricer_hpp
#define quantlib_cmsspread_pricer_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/indexes/swapspreadindex.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

namespace QuantLib {

    class CmsSpreadCoupon;
    class YieldTermStructure;

    //! CMS spread - coupon pricer
    /*! blah blah...
    */

    class CmsSpreadPricer : public CmsSpreadCouponPricer {

      public:

        CmsSpreadPricer(const boost::shared_ptr<CmsCouponPricer> cmsPricer,
                        const Handle<Quote> &correlation,
                        const Handle<YieldTermStructure> &couponDiscountCurve =
                            Handle<YieldTermStructure>(),
                        const Size IntegrationPoints = 32);

        /* */
        virtual Real swapletPrice() const;
        virtual Rate swapletRate() const;
        virtual Real capletPrice(Rate effectiveCap) const;
        virtual Rate capletRate(Rate effectiveCap) const;
        virtual Real floorletPrice(Rate effectiveFloor) const;
        virtual Rate floorletRate(Rate effectiveFloor) const;
        /* */
        void flushCache();

      private:

        typedef std::map<std::pair<std::string,Date>,std::pair<Real,Real> > CacheType;


        void initialize(const FloatingRateCoupon &coupon);
        Real optionletPrice(Option::Type optionType, Real strike) const;
       
        const Real integrand(const Real) const;
        
        boost::shared_ptr<CmsCouponPricer> cmsPricer_;

        Handle<YieldTermStructure> couponDiscountCurve_;

        const CmsSpreadCoupon *coupon_;

        Date today_, fixingDate_, paymentDate_;

        Real fixingTime_;

        Real gearing_, spread_;
        Real spreadLegValue_;

        boost::shared_ptr<SwapSpreadIndex> index_;

        boost::shared_ptr<CumulativeNormalDistribution> cnd_;
        boost::shared_ptr<GaussHermiteIntegration> integrator_;

        Real swapRate1_, swapRate2_, gearing1_, gearing2_;
        Real adjustedRate1_, adjustedRate2_;
        Real vol1_, vol2_;
        mutable Real phi_, strike_;
        Real mu1_,mu2_;
        Real rho_;

        boost::shared_ptr<CmsCoupon> c1_, c2_;

        CacheType cache_;

    };
}

#endif