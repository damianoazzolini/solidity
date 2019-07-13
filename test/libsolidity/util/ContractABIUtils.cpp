/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <test/libsolidity/util/ContractABIUtils.h>

#include <test/libsolidity/util/SoltestErrors.h>

#include <liblangutil/Common.h>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/range/algorithm_ext/for_each.hpp>

#include <fstream>
#include <memory>
#include <numeric>
#include <regex>
#include <stdexcept>

using namespace dev;
using namespace langutil;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace soltest;

namespace
{

using ParameterList = dev::solidity::test::ParameterList;

auto arraySize(string const& _arrayType) -> size_t
{
	auto leftBrack = _arrayType.find("[");
	auto rightBrack = _arrayType.find("]");

	soltestAssert(leftBrack != string::npos && rightBrack != string::npos, "");

	string size = _arrayType.substr(leftBrack + 1, rightBrack - leftBrack - 1);
	return static_cast<size_t>(stoi(size));
}

auto isBool(string _type) -> bool
{
	return regex_match(_type, regex{"(bool)"});
}

auto isUint(string _type) -> bool
{
	return regex_match(_type, regex{"(uint\\d*)"});
}

auto isInt(string _type) -> bool
{
	return regex_match(_type, regex{"(int\\d*)"});
}

auto isBytes(string _type) -> bool
{
	return regex_match(_type, regex{"(bytes\\d+)"});
}

auto isDynBytes(string _type) -> bool
{
	return regex_match(_type, regex{"(\\bbytes\\b)"});
}

auto isString(string _type) -> bool
{
	return regex_match(_type, regex{"(string)"});
}

auto isBoolArray(string _type) -> bool
{
	return regex_match(_type, regex{"(bool)(\\[\\d+\\])"});
}

auto isUintArray(string _type) -> bool
{
	return regex_match(_type, regex{"(uint\\d*)(\\[\\d+\\])"});
}

auto isIntArray(string _type) -> bool
{
	return regex_match(_type, regex{"(int\\d*)(\\[\\d+\\])"});
}

auto isStringArray(string _type) -> bool
{
	return regex_match(_type, regex{"(string)(\\[\\d+\\])"});
}

auto isTuple(string _type) -> bool
{
	return regex_match(_type, regex{"(tuple)"});
}

auto isTupleArray(string _type) -> bool
{
	return regex_match(_type, regex{"(tuple)(\\[\\d+\\])"});
}

auto functionMatchesSignature(Json::Value const& _contractABIFunction, string const& _signature) -> bool
{
	string functionName{_signature.substr(0, _signature.find("("))};
	string functionParameters{_signature.substr(_signature.find("(") + 1, _signature.find(")") - 2)};

	vector<string> parameterTypes;
	if (!functionParameters.empty())
		boost::split(parameterTypes, functionParameters, [](char c){return c == ',';});

	vector<string> abiParameterTypes;
	for (auto const& input: _contractABIFunction["inputs"])
		abiParameterTypes.push_back(input["type"].asString());

	return (
		_contractABIFunction["name"] == functionName &&
		parameterTypes == abiParameterTypes
	);
}

}

boost::optional<dev::solidity::test::ParameterList> ContractABIUtils::parametersFromJson(
	ErrorReporter& _errorReporter,
	Json::Value const& _contractABI,
	string const& _functionSignature
)
{
	ParameterList addressTypeParams;
	ParameterList valueTypeParams;
	ParameterList dynamicTypeParams;

	ParameterList finalParams;

	if (!_contractABI)
		return boost::none;

	for (auto const& function: _contractABI)
	{
		if (functionMatchesSignature(function, _functionSignature))
			for (auto const& output: function["outputs"])
			{
				string type = output["type"].asString();

				ABITypes addressTypes;
				ABITypes valueTypes;
				ABITypes dynamicTypes;

				if (appendTypesFromName(output, addressTypes, valueTypes, dynamicTypes))
				{
					for (auto const& type: addressTypes)
						addressTypeParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
					for (auto const& type: valueTypes)
						valueTypeParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
					for (auto const& type: dynamicTypes)
						dynamicTypeParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
				}
				else
				{
					_errorReporter.warning(
						"Could not convert \"" + type +
						"\" to internal ABI type representation. Falling back to default encoding."
					);
					return boost::none;
				}

				finalParams += addressTypeParams + valueTypeParams;

				addressTypeParams.clear();
				valueTypeParams.clear();
			}
	}

	return boost::optional<ParameterList>(finalParams + dynamicTypeParams);
}

bool ContractABIUtils::appendTypesFromName(
	Json::Value const& _functionOutput,
	ABITypes& _addressTypes,
	ABITypes& _valueTypes,
	ABITypes& _dynamicTypes,
	bool _isCompoundType
)
{
	string type = _functionOutput["type"].asString();
	if (isBool(type))
		_valueTypes.push_back(ABIType{ABIType::Boolean});
	else if (isUint(type))
		_valueTypes.push_back(ABIType{ABIType::UnsignedDec});
	else if (isInt(type))
		_valueTypes.push_back(ABIType{ABIType::SignedDec});
	else if (isBytes(type))
		_valueTypes.push_back(ABIType{ABIType::Hex});
	else if (isString(type))
	{
		_addressTypes.push_back(ABIType{ABIType::Hex});

		if (_isCompoundType)
			_dynamicTypes.push_back(ABIType{ABIType::Hex});

		_dynamicTypes.push_back(ABIType{ABIType::UnsignedDec});
		_dynamicTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft});
	}
	else if (isTuple(type))
	{
		for (auto const& component: _functionOutput["components"])
			appendTypesFromName(component, _addressTypes, _valueTypes, _dynamicTypes, true);
	}
	else if (isBoolArray(type))
		fill_n(back_inserter(_valueTypes), arraySize(type), ABIType{ABIType::Boolean});
	else if (isUintArray(type))
		fill_n(back_inserter(_valueTypes), arraySize(type), ABIType{ABIType::UnsignedDec});
	else if (isIntArray(type))
		fill_n(back_inserter(_valueTypes), arraySize(type), ABIType{ABIType::SignedDec});
	else if (isStringArray(type))
	{
		_addressTypes.push_back(ABIType{ABIType::Hex});

		fill_n(back_inserter(_dynamicTypes), arraySize(type), ABIType{ABIType::Hex});

		for (size_t i = 0; i < arraySize(type); i++)
		{
			_dynamicTypes.push_back(ABIType{ABIType::UnsignedDec});
			_dynamicTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft});
		}
	}
	else if (isDynBytes(type))
		return false;
	else if (isTupleArray(type))
		return false;
	else
		return false;

	return true;
}

void ContractABIUtils::overwriteParameters(
	ErrorReporter& _errorReporter,
	dev::solidity::test::ParameterList& _targetParameters,
	dev::solidity::test::ParameterList const& _sourceParameters
)
{
	auto overwrite = [&](Parameter _a, Parameter& _b) -> void
	{
		if (
			_a.abiType.size != _b.abiType.size ||
			_a.abiType.type != _b.abiType.type
		)
		{
			_errorReporter.warning("Type or size of parameter(s) does not match.");
			_b = _a;
		}
	};
	boost::for_each(_sourceParameters, _targetParameters, boost::bind<void>(overwrite, _1, _2));
}

dev::solidity::test::ParameterList ContractABIUtils::preferredParameters(
	ErrorReporter& _errorReporter,
	dev::solidity::test::ParameterList const& _targetParameters,
	dev::solidity::test::ParameterList const& _sourceParameters,
	bytes _bytes
)
{
	if (_targetParameters.size() != _sourceParameters.size())
	{
		auto sizeFold = [](size_t const _a, Parameter const& _b) { return _a + _b.abiType.size; };
		size_t encodingSize = accumulate(_targetParameters.begin(), _targetParameters.end(), size_t{0}, sizeFold);

		_errorReporter.warning(
			"Encoding does not match byte range. The call returned " +
			to_string(_bytes.size()) + " bytes, but " +
			to_string(encodingSize) + " bytes were expected."
		);
		return _sourceParameters;
	}
	else
		return _targetParameters;
}
