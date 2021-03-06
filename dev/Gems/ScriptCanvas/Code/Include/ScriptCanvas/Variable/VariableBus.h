/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Outcome/Outcome.h>

#include <ScriptCanvas/Variable/VariableDatum.h>

namespace ScriptCanvas
{
    class VariableData;

    using GraphVariableMapping = AZStd::unordered_map< VariableId, VariableNameValuePair >;

    // Bus Interface for adding, removing and finding exposed Variable datums associated with a ScriptCanvas Graph
    class VariableRequests
    {
    public:
        using AllocatorType = AZStd::allocator;

        //! Retrieves address of the variable datum that can be modified
        virtual VariableDatum* GetVariableDatum() = 0;
        virtual const VariableDatum* GetVariableDatumConst() const = 0;

        //! Returns the type associated with the specified variable.
        virtual Data::Type GetType() const = 0;

        //! Looks up the variable name that the variable id is associated with in the handler of the bus
        virtual AZStd::string_view GetName() const = 0;


        //! Changes the name of the variable with the specified @variableId within the handler 
        //! returns an AZ::Outcome to indicate if the variable was able to be succesfully or an error message to indicate
        //! why the rename failed
        virtual AZ::Outcome<void, AZStd::string> RenameVariable(AZStd::string_view newVarName) = 0;
    };

    class CopiedVariableData
    {
    public:
        typedef AZStd::unordered_map< VariableId, VariableNameValuePair > VariableMapping;

        AZ_RTTI(CopiedVariableData, "{84548415-DD9E-4943-8D1E-3E1CC49ADACB}");
        AZ_CLASS_ALLOCATOR(CopiedVariableData, AZ::SystemAllocator, 0);

        virtual ~CopiedVariableData() = default;

        static const char* k_variableKey;
        VariableMapping m_variableMapping;
    };

    class GraphVariableManagerRequests
    {
    public:
        using AllocatorType = AZStd::allocator;

        //! Adds a variable that is keyed by the string and maps to a type that can be storedAZStd::any(any type with a AzTypeInfo specialization)
        //! returns an AZ::Outcome which on success contains the VariableId and on Failure contains a string with error information
        virtual AZ::Outcome<VariableId, AZStd::string> RemapVariable(const VariableNameValuePair& variableConfiguration) = 0;
        virtual AZ::Outcome<VariableId, AZStd::string> AddVariable(AZStd::string_view key, const Datum& value) = 0;
        virtual AZ::Outcome<VariableId, AZStd::string> AddVariablePair(const AZStd::pair<AZStd::string_view, Datum>& keyValuePair) = 0;

        //! Adds properties from the range [first, last)
        //! returns vector of AZ::Outcome which for successful outcomes contains the VariableId and for failing outcome
        //! contains string detailing the reason for failing to add the variable
        template<typename InputIt>
        AZStd::vector<AZ::Outcome<VariableId, AZStd::string>> AddVariables(InputIt first, InputIt last)
        {
            static_assert(AZStd::is_same<typename AZStd::iterator_traits<InputIt>::value_type, AZStd::pair<AZStd::string_view, Datum>>::value, "Only iterators to pair<string_view, any> are supported");

            AZStd::vector<AZ::Outcome<VariableId, AZStd::string>> addVariableOutcomes;
            for (; first != last; ++first)
            {
                addVariableOutcomes.push_back(AddVariablePair(*first));
            }

            return addVariableOutcomes;
        }

        //! Remove a single variable which matches the specified variable id
        //! returns true if the variable with the variable id was removed
        virtual bool RemoveVariable(const VariableId&) = 0;
        //! Removes properties which matches the specified string name
        //! returns the number of properties removed
        virtual AZStd::size_t RemoveVariableByName(AZStd::string_view) = 0;
        //! Removes properties which matches the specified variable ids
        //! returns the number of properties removed
        template<typename InputIt>
        AZStd::size_t RemoveVariables(InputIt first, InputIt last)
        {
            AZStd::size_t removedVariableCount = 0U;
            static_assert(AZStd::is_same<typename AZStd::iterator_traits<InputIt>::value_type, VariableId>::value, "Only input iterators to VariableId are supported");
            for (; first != last; ++first)
            {
                removedVariableCount += RemoveVariable(*first) ? 1 : 0;
            }

            return removedVariableCount;
        }

        //! Searches for a variable with the specified name
        //! returns pointer to the first variable with the specified name or nullptr
        virtual VariableDatum* FindVariable(AZStd::string_view propName) = 0;        

        //! Returns the type associated with the specified variable.
        virtual Data::Type GetVariableType(const VariableId& variableId) = 0;

        //! Searches for a variable by VariableId
        //! returns a pair of <variable datum pointer, variable name> with the supplied id
        //! The variable datum pointer is non-null if the variable has been found
        virtual VariableNameValuePair* FindVariableById(const VariableId& varId) = 0;

        //! Retrieves all properties stored by the Handler
        //! returns variable container
        virtual const GraphVariableMapping* GetVariables() const = 0;

        //! Looks up the variable name that the variable data is associated with in the handler of the bus
        virtual AZStd::string_view GetVariableName(const VariableId&) const = 0;
        //! Changes the name of the variable with the specified @variableId within the handler 
        //! returns an AZ::Outcome to indicate if the variable was able to be succesfully or an error message to indicate
        //! why the rename failed
        virtual AZ::Outcome<void, AZStd::string> RenameVariable(const VariableId& variableId, AZStd::string_view newVarName) = 0;

        virtual const VariableData* GetVariableDataConst() const = 0;
        virtual VariableData* GetVariableData() = 0;
        //! Sets the VariableData and connects the variables ids to the VariableRequestBus for this handler
        virtual void SetVariableData(const VariableData& variableData) = 0;
        //! Deletes oldVariableData and sends out GraphVariableManagerNotifications for each deleted variable
        virtual void DeleteVariableData(const VariableData& variableData) = 0;
    };

    class VariableNodeRequests
    {
    public:
        // Sets the VariableId on a node that interfaces with a variable(i.e the GetVariable and SetVariable node)
        virtual void SetId(const VariableId& variableId) = 0;
        // Retrieves the VariableId on a node that interfaces with a variable(i.e the GetVariable and SetVariable node)
        virtual const VariableId& GetId() const = 0;
    };

    struct RequestByVariableIdTraits : public AZ::EBusTraits
    {
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = VariableId;
    };

    struct RequestByGraphIdTraits : public AZ::EBusTraits
    {
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
    };

    struct RequestByNodeIdTraits : public AZ::EBusTraits
    {
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
    };

    using VariableRequestBus = AZ::EBus<VariableRequests, RequestByVariableIdTraits>;
    using GraphVariableManagerRequestBus = AZ::EBus<GraphVariableManagerRequests, RequestByGraphIdTraits>;
    using VariableNodeRequestBus = AZ::EBus<VariableNodeRequests, RequestByNodeIdTraits>;

    class GraphVariableManagerNotifications
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;

        // Invoked when after a variable has been added to the handler
        virtual void OnVariableAdded(const ScriptCanvas::VariableId& /*variableId*/, AZStd::string_view /*variableName*/) {}
        // Invoked after a variable has been removed from the handler
        virtual void OnVariableRemoved(const ScriptCanvas::VariableId& /*variableId*/, AZStd::string_view /*variableName*/) {}
        // Invoked after a variable has been renamed
        virtual void OnVariableNameChanged(const ScriptCanvas::VariableId& /*variableId*/, AZStd::string_view /*variableName*/) {}
        // Invoked after the variable data has been set on the variable handler
        virtual void OnVariableDataSet() {}
    };

    using GraphVariableManagerNotificationBus = AZ::EBus<GraphVariableManagerNotifications>;

    class VariableNotifications
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = VariableId;

        // Invoked before a variable is erased from the Variable Bus Handler
        virtual void OnVariableRemoved() {}

        // Invoked after a variable is renamed
        virtual void OnVariableRenamed(AZStd::string_view /*newVariableName*/) {}

        virtual void OnVariableValueChanged() {};

        virtual void OnVariableExposureChanged() {};
    };

    using VariableNotificationBus = AZ::EBus<VariableNotifications>;

    class VariableNodeNotifications
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
        using BusIdType = AZ::EntityId;
        // Invoked after the variable id has been changed on the SetVariable/GetVariableNode
        virtual void OnVariableIdChanged(const VariableId& /*oldVariableId*/, const VariableId& /*newVariableId*/) {}
        // Invoked after the variable has been removed from the GraphVariableManagerRequestBus
        virtual void OnVariableRemoved(const VariableId& /*removedVariableId*/) {}
    };

    using VariableNodeNotificationBus = AZ::EBus<VariableNodeNotifications>;
}