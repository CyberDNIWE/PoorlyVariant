
// Can define different typeid underlying type if needed
//#define POORLY_TYPEID_T unsigned char

#include "PoorlyVariant.h"

#include <iostream>
#include <string>

// Case for times when only one concrete visitor is needed, also modifies the value
struct MyVariantVisitsConcrete
{
	void operator()(const int& i)
	{
		std::cout << "MyVariantVisitsConcrete::operator() called with const int:    " << i << std::endl;
	}

	void operator()(const double& d)
	{
		std::cout << "MyVariantVisitsConcrete::operator() called with const double: " << d << std::endl;
	}
};



// Case for polymorphic visitors. No forced inheritance, if only concrete visitor class needs acceptance
struct VisitorForSpecificType_int
{
	virtual void operator()(int&)
	{ /* throw here or just ignore */
	};
};

struct VisitorForSpecificType_double
{
	virtual void operator()(double&)
	{ /* throw here or just ignore */
	};
};

struct VisitorForSpecificTypesBase : public VisitorForSpecificType_int, public VisitorForSpecificType_double
{
	// Or can be just
	/*

	virtual void operator()(int&)
	{  throw here or just ignore  };

	virtual void operator()(double&)
	{ throw here or just ignore };

	//*/
};


struct MyVariantVisitsPolymorphically : public VisitorForSpecificTypesBase
{
	using visitor_base_type = VisitorForSpecificTypesBase;

	virtual void operator()(int& i) override
	{
		std::cout << "MyVariantVisitsPolymorphically::operator() called with int:    " << i << std::endl;
	}

	virtual void operator()(double& d) override
	{
		std::cout << "MyVariantVisitsPolymorphically::operator() called with double: " << d << std::endl;
	}
};

struct MyVariantVisitsPolymorphically2 : public VisitorForSpecificTypesBase
{
	using visitor_base_type = VisitorForSpecificTypesBase;

	virtual void operator()(int& i) override
	{
		std::cout << "MyVariantVisitsPolymorphically2::operator() called with int:    "
			<< i << " which will be multiplied by 2" << std::endl;
		i *= 2;
	}

	virtual void operator()(double& d) override
	{
		std::cout << "MyVariantVisitsPolymorphically2::operator() called with double: "
			<< d << " which will be divided by 2" << std::endl;
		d /= 2;
	}
};




int main()
{
	/*			  Basic use of variant	    */
	using MyVariant = poorly::variant<int, double>;
	MyVariant mySimpleVariant = {};

	// Always set with explicit type
	mySimpleVariant.set<int>(1);

	//mySimpleVariant.set<float>(1.f); // Will fail to compile since only <int, double> expected

	// Get by type -> check if returned ptr not null	
	auto* i_ptr = mySimpleVariant.get<int>();
	auto i = i_ptr ? *i_ptr : 0; // i will be 1

/*			  Concrete visitor 		    */
	using MyVariantVisitable = poorly::variant_visitable<MyVariantVisitsConcrete, int, double>;
	MyVariantVisitable variantVisitableConcrete = {};


	// Store value into variant
	variantVisitableConcrete.set<int>(2);

	// Basic use of variant: query by type -> check if returned ptr not null
	i_ptr = variantVisitableConcrete.get<int>();
	i = i_ptr ? *i_ptr : 0;

	// Make visitor and visit
	MyVariantVisitsConcrete visitorConcrete = {};
	variantVisitableConcrete.accept_visitor(visitorConcrete);

	// Store new value
	variantVisitableConcrete.set<double>(3.14);
	// Get that value for before- comaring
	auto* d_ptr = variantVisitableConcrete.get<double>();
	auto d = d_ptr ? *d_ptr : 0.0;
	// Accept visitor again
	variantVisitableConcrete.accept_visitor(visitorConcrete);

	std::cout << std::endl;

	/*			Polymorphic visitor(s)		*/

	using MyVariantPolyVisitable = poorly::variant_visitable<MyVariantVisitsPolymorphically::visitor_base_type, int, double>;
	MyVariantPolyVisitable hvariant = {};
	hvariant.set<int>(2);

	// Basic use of variant: query by type -> check if returned ptr not null
	i_ptr = hvariant.get<int>();
	i = i_ptr ? *i_ptr : 0;

	// Make visitor and visit
	MyVariantVisitsPolymorphically visitorPolymorphic = {};
	hvariant.accept_visitor(visitorPolymorphic);

	// Store another value
	hvariant.set<double>(3.14);

	// Get that value for before- comaring
	d_ptr = hvariant.get<double>();
	d = d_ptr ? *d_ptr : 0.0;

	std::cout << "Before getting visited by second polymorphic visitor the holded value is: " << d << std::endl;

	// Make another visitor and visit
	MyVariantVisitsPolymorphically2 visitorPolymorphic2 = {};
	hvariant.accept_visitor(visitorPolymorphic2);

	// Get the value for after- comparing
	d_ptr = hvariant.get<double>();
	d = d_ptr ? *d_ptr : 0.0;
	std::cout << "After  getting visited by second polymorphic visitor the holded value is: " << d << std::endl;



	std::getchar();
	return 0;
}