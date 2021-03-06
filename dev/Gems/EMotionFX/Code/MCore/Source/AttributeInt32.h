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

// include the required headers
#include "StandardHeaders.h"
#include "Attribute.h"
#include "StringConversions.h"

namespace MCore
{
    /**
     * The signed 32 bit integer attribute class.
     * This attribute represents one signed int.
     */
    class MCORE_API AttributeInt32
        : public Attribute
    {
        friend class AttributeFactory;
    public:
        enum
        {
            TYPE_ID = 0x00000002
        };

        static AttributeInt32* Create(int32 value = 0);

        // adjust values
        MCORE_INLINE int32 GetValue() const                         { return mValue; }
        MCORE_INLINE void SetValue(int32 value)                     { mValue = value; }

        MCORE_INLINE uint8* GetRawDataPointer()                     { return reinterpret_cast<uint8*>(&mValue); }
        MCORE_INLINE uint32 GetRawDataSize() const                  { return sizeof(int32); }
        bool GetSupportsRawDataPointer() const override             { return true; }

        // overloaded from the attribute base class
        Attribute* Clone() const override                           { return AttributeInt32::Create(mValue); }
        Attribute* CreateInstance(void* destMemory) override        { return new(destMemory) AttributeInt32(); }
        const char* GetTypeString() const override                  { return "AttributeInt32"; }
        bool InitFrom(const Attribute* other) override
        {
            if (other->GetType() != TYPE_ID)
            {
                return false;
            }
            mValue = static_cast<const AttributeInt32*>(other)->GetValue();
            return true;
        }
        bool InitFromString(const AZStd::string& valueString) override
        {
            return AzFramework::StringFunc::LooksLikeInt(valueString.c_str(), &mValue);
        }
        bool ConvertToString(AZStd::string& outString) const override      { outString = AZStd::string::format("%d", mValue); return true; }
        uint32 GetClassSize() const override                        { return sizeof(AttributeInt32); }
        uint32 GetDefaultInterfaceType() const override             { return ATTRIBUTE_INTERFACETYPE_INTSPINNER; }
        void Scale(float scaleFactor) override                      { mValue = (int32)(mValue * scaleFactor); }

    private:
        int32   mValue;     /**< The signed integer value. */

        AttributeInt32()
            : Attribute(TYPE_ID)
            , mValue(0)  {}
        AttributeInt32(int32 value)
            : Attribute(TYPE_ID)
            , mValue(value) {}
        ~AttributeInt32() {}

        uint32 GetDataSize() const override                         { return sizeof(int32); }

        // read from a stream
        bool ReadData(MCore::Stream* stream, MCore::Endian::EEndianType streamEndianType, uint8 version) override
        {
            MCORE_UNUSED(version);

            int32 streamValue;
            if (stream->Read(&streamValue, sizeof(int32)) == 0)
            {
                return false;
            }

            Endian::ConvertSignedInt32(&streamValue, streamEndianType);
            mValue = streamValue;
            return true;
        }


        // write to a stream
        bool WriteData(MCore::Stream* stream, MCore::Endian::EEndianType targetEndianType) const override
        {
            int32 streamValue = mValue;
            Endian::ConvertSignedInt32To(&streamValue, targetEndianType);
            if (stream->Write(&streamValue, sizeof(int32)) == 0)
            {
                return false;
            }

            return true;
        }
    };
}   // namespace MCore
