/*    Copyright 2009 10gen Inc.
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/

#pragma once

#include <map>
#include <cmath>
#include <limits>
#include "bsontypes.h"
#include "parse_number.h"
#include "BsonElement.h"
#include "BsonObject.h"
#include "builder.h"
#include "oid.h"
#include "time_support.h"

namespace Bson {

#if defined(_WIN32)
    // warning: 'this' : used in base member initializer list
#pragma warning( disable : 4355 )
#endif

    class BsonIterator;

    /** Utility for creating a BsonObject.
    See also the BSON() and BSON_ARRAY() macros.
    */
    class BsonObjectBuilder {
        BufBuilder &_b;
        BufBuilder _buf;
        int _offset;
        bool _doneCalled;

    public:
        char* _done() {
            if (_doneCalled)
                return _b.buf() + _offset;

            _doneCalled = true;
            //_s.endField();
            _b.appendNum((char)EOO);
            char *data = _b.buf() + _offset;
            int size = _b.len() - _offset;
            *((int*)data) = endian_int(size);
            return data;
        }

        /** @param initsize this is just a hint as to the final size of the object */
        BsonObjectBuilder(int initsize = 512) : _b(_buf), _buf(initsize + sizeof(unsigned)), _offset(0), _doneCalled(false) {
            _b.skip(4); /*leave room for size field and ref-count*/
        }

        /** @param baseBuilder construct a BsonObjectBuilder using an existing BufBuilder
        *  This is for more efficient adding of subobjects/arrays. See docs for subobjStart for example.
        */
        BsonObjectBuilder(BufBuilder &baseBuilder) : _b(baseBuilder), _buf(0), _offset(baseBuilder.len()), _doneCalled(false) {
            _b.skip(4);
        }

        ~BsonObjectBuilder() {
            if (!_doneCalled && _b.buf() && _buf.getSize() == 0) {
                _done();
            }
        }

        /** add all the fields from the object specified to this object */
        BsonObjectBuilder& appendElements(BsonObject x);

        /** add all the fields from the object specified to this object if they don't exist already */
        BsonObjectBuilder& appendElementsUnique(BsonObject x);

        /** append element to the object we are building */
        BsonObjectBuilder& append(const BsonElement& e) {
            verify(!e.eoo()); // do not append eoo, that would corrupt us. the builder auto appends when done() is called.
            _b.appendBuf((void*)e.rawdata(), e.size());
            return *this;
        }

        /** append an element but with a new name */
        BsonObjectBuilder& appendAs(const BsonElement& e, const StringData& fieldName) {
            verify(!e.eoo()); // do not append eoo, that would corrupt us. the builder auto appends when done() is called.
            _b.appendNum((char)e.type());
            _b.appendStr(fieldName);
            _b.appendBuf((void *)e.value(), e.valuesize());
            return *this;
        }

        /** add a subobject as a member */
        BsonObjectBuilder& append(const StringData& fieldName, BsonObject subObj) {
            _b.appendNum((char)Object);
            _b.appendStr(fieldName);
            _b.appendBuf((void *)subObj.objdata(), subObj.objsize());
            return *this;
        }

        /** add a subobject as a member */
        BsonObjectBuilder& appendObject(const StringData& fieldName, const char * objdata, int size = 0) {
            verify(objdata != 0);
            if (size == 0) {
                size = *((int*)objdata);
            }

            verify(size > 4 && size < 100000000);

            _b.appendNum((char)Object);
            _b.appendStr(fieldName);
            _b.appendBuf((void*)objdata, size);
            return *this;
        }

        /** add a subobject as a member with type Array.  Thus arr object should have "0", "1", ...
        style fields in it.
        */
        BsonObjectBuilder& appendArray(const StringData& fieldName, const BsonObject &subObj) {
            _b.appendNum((char)Array);
            _b.appendStr(fieldName);
            _b.appendBuf((void *)subObj.objdata(), subObj.objsize());
            return *this;
        }

        /** add header for a new subarray and return bufbuilder for writing to
            the subarray's body 
        
            e.g.:
              BufBuilder& sub = b.subarrayStart("myArray");
              sub.append( "0", "hi" );
              sub.append( "1", "there" );
              sub.append( "2", 33 );
              sub._done();

        */
        BufBuilder &subarrayStart(const StringData& fieldName) {
            _b.appendNum((char)Array);
            _b.appendStr(fieldName);
            return _b;
        }

        /** Append a boolean element */
        BsonObjectBuilder& appendBool(const StringData& fieldName, int val) {
            _b.appendNum((char)Bool);
            _b.appendStr(fieldName);
            _b.appendNum((char)(val ? 1 : 0));
            return *this;
        }

        /** Append a boolean element */
        BsonObjectBuilder& append(const StringData& fieldName, bool val) {
            _b.appendNum((char)Bool);
            _b.appendStr(fieldName);
            _b.appendNum((char)(val ? 1 : 0));
            return *this;
        }

        /** Append a 32 bit integer element */
        BsonObjectBuilder& append(const StringData& fieldName, int n) {
            _b.appendNum((char)NumberInt);
            _b.appendStr(fieldName);
            _b.appendNum(n);
            return *this;
        }

        /** Append a 32 bit unsigned element - cast to a signed int. */
        BsonObjectBuilder& append(const StringData& fieldName, unsigned n) {
            return append(fieldName, (int)n);
        }

        /** Append a NumberLong */
        BsonObjectBuilder& append(const StringData& fieldName, long long n) {
            _b.appendNum((char)NumberLong);
            _b.appendStr(fieldName);
            _b.appendNum(n);
            return *this;
        }

        /** appends a number.  if n < max(int)/2 then uses int, otherwise long long */
        BsonObjectBuilder& appendIntOrLL(const StringData& fieldName, long long n) {
            // extra () to avoid max macro on windows
            static const long long maxInt = (std::numeric_limits<int>::max)() / 2;
            static const long long minInt = -maxInt;
            if (minInt < n && n < maxInt) {
                append(fieldName, static_cast<int>(n));
            }
            else {
                append(fieldName, n);
            }
            return *this;
        }

        /**
        * appendNumber is a series of method for appending the smallest sensible type
        * mostly for JS
        */
        BsonObjectBuilder& appendNumber(const StringData& fieldName, int n) {
            return append(fieldName, n);
        }

        BsonObjectBuilder& appendNumber(const StringData& fieldName, double d) {
            return append(fieldName, d);
        }

        BsonObjectBuilder& appendNumber(const StringData& fieldName, size_t n) {
            static const size_t maxInt = (1 << 30);

            if (n < maxInt)
                append(fieldName, static_cast<int>(n));
            else
                append(fieldName, static_cast<long long>(n));
            return *this;
        }

        BsonObjectBuilder& appendNumber(const StringData& fieldName, long long llNumber) {
            static const long long maxInt = (1LL << 30);
            static const long long minInt = -maxInt;
            static const long long maxDouble = (1LL << 40);
            static const long long minDouble = -maxDouble;

            if (minInt < llNumber && llNumber < maxInt) {
                append(fieldName, static_cast<int>(llNumber));
            }
            else if (minDouble < llNumber && llNumber < maxDouble) {
                append(fieldName, static_cast<double>(llNumber));
            }
            else {
                append(fieldName, llNumber);
            }

            return *this;
        }

        /** Append a double element */
        BsonObjectBuilder& append(const StringData& fieldName, double n) {
            _b.appendNum((char)NumberDouble);
            _b.appendStr(fieldName);
            _b.appendNum(n);
            return *this;
        }

        /** tries to append the data as a number
        * @return true if the data was able to be converted to a number
        */
        bool appendAsNumber(const StringData& fieldName, const std::string& data);

        /**
        Append a BSON Object ID.
        @param fieldName Field name, e.g., "_id".
        @returns the builder object
        */
        BsonObjectBuilder& append(const StringData& fieldName, OID oid) {
            _b.appendNum((char)jstOID);
            _b.appendStr(fieldName);
            _b.appendBuf((void *)&oid, 12);
            return *this;
        }

        /** Append a time_t date.
        @param dt a C-style 32 bit date value, that is
        the number of seconds since January 1, 1970, 00:00:00 GMT
        */
        BsonObjectBuilder& appendTimeT(const StringData& fieldName, time_t dt) {
            _b.appendNum((char)Date);
            _b.appendStr(fieldName);
            _b.appendNum(static_cast<unsigned long long>(dt)* 1000);
            return *this;
        }
        /** Append a date.
        @param dt a Java-style 64 bit date value, that is
        the number of milliseconds since January 1, 1970, 00:00:00 GMT
        */
        BsonObjectBuilder& appendDate(const StringData& fieldName, Date_t dt) {
            /* easy to pass a time_t to this and get a bad result.  thus this warning. */
#if defined(_DEBUG) && defined(MONGO_EXPOSE_MACROS)
            if (dt > 0 && dt <= 0xffffffff) {
                static int n;
                if (n++ == 0)
                    log() << "DEV WARNING appendDate() called with a tiny (but nonzero) date" << std::endl;
            }
#endif
            _b.appendNum((char)Date);
            _b.appendStr(fieldName);
            _b.appendNum(dt);
            return *this;
        }
        BsonObjectBuilder& append(const StringData& fieldName, Date_t dt) {
            return appendDate(fieldName, dt);
        }


        /** Append a regular expression value
            @param regex the regular expression pattern
            @param regex options such as "i" or "g"
        */
        BsonObjectBuilder& appendRegex(const StringData& fieldName, const StringData& regex, const StringData& options = "") {
            _b.appendNum((char) RegEx);
            _b.appendStr(fieldName);
            _b.appendStr(regex);
            _b.appendStr(options);
            return *this;
        }

        BsonObjectBuilder& appendCode(const StringData& fieldName, const StringData& code) {
            _b.appendNum((char) Code);
            _b.appendStr(fieldName);
            _b.appendNum((int) code.size()+1);
            _b.appendStr(code);
            return *this;
        }
        /** Append a string element.
            @param sz size includes terminating null character */
        BsonObjectBuilder& append(const StringData& fieldName, const char *str, int sz) {
            _b.appendNum((char) String);
            _b.appendStr(fieldName);
            _b.appendNum((int)sz);
            _b.appendBuf(str, sz);
            return *this;
        }
        /** Append a string element */
        BsonObjectBuilder& append(const StringData& fieldName, const char *str) {
            return append(fieldName, str, (int) strlen(str)+1);
        }
        /** Append a string element */
        BsonObjectBuilder& append(const StringData& fieldName, const std::string& str) {
            return append(fieldName, str.c_str(), (int) str.size()+1);
        }
        /** Append a string element */
        BsonObjectBuilder& append(const StringData& fieldName, const StringData& str) {
            _b.appendNum((char) String);
            _b.appendStr(fieldName);
            _b.appendNum((int)str.size()+1);
            _b.appendStr(str, true);
            return *this;
        }
        BsonObjectBuilder& appendSymbol(const StringData& fieldName, const StringData& symbol) {
            _b.appendNum((char) Symbol);
            _b.appendStr(fieldName);
            _b.appendNum((int) symbol.size()+1);
            _b.appendStr(symbol);
            return *this;
        }

        /** Implements builder interface but no-op in ObjBuilder */
        void appendNull() {
            msgasserted(16234, "Invalid call to appendNull in BsonObject Builder.");
        }

        /** Append a Null element to the object */
        BsonObjectBuilder& appendNull( const StringData& fieldName ) {
            _b.appendNum( (char) jstNULL );
            _b.appendStr( fieldName );
            return *this;
        }

        // Append an element that is less than all other keys.
        BsonObjectBuilder& appendMinKey( const StringData& fieldName ) {
            _b.appendNum( (char) MinKey );
            _b.appendStr( fieldName );
            return *this;
        }
        // Append an element that is greater than all other keys.
        BsonObjectBuilder& appendMaxKey( const StringData& fieldName ) {
            _b.appendNum( (char) MaxKey );
            _b.appendStr( fieldName );
            return *this;
        }
        // Append a Timestamp field -- will be updated to next OpTime on db insert.
        BsonObjectBuilder& appendTimestamp( const StringData& fieldName ) {
            _b.appendNum( (char) Timestamp );
            _b.appendStr( fieldName );
            _b.appendNum( (unsigned long long) 0 );
            return *this;
        }

        /** add header for a new subobject and return bufbuilder for writing to
         *  the subobject's body
         *
         *  example:
         *
         *  BsonObjectBuilder b;
         *  BsonObjectBuilder sub (b.subobjStart("fieldName"));
         *  // use sub
         *  sub.done()
         *  // use b and convert to object
         */
        BufBuilder &subobjStart(const StringData& fieldName) {
            _b.appendNum((char) Object);
            _b.appendStr(fieldName);
            return _b;
        }
        
        /**
         * Alternative way to store an OpTime in BSON. Pass the OpTime as a Date, as follows:
         *
         *     builder.appendTimestamp("field", optime.asDate());
         *
         * This captures both the secs and inc fields.
         */
        BsonObjectBuilder& appendTimestamp( const StringData& fieldName , unsigned long long val ) {
            _b.appendNum( (char) Timestamp );
            _b.appendStr( fieldName );
            _b.appendNum( val );
            return *this;
        }

        /**
        Timestamps are a special BSON datatype that is used internally for replication.
        Append a timestamp element to the object being ebuilt.
        @param time - in millis (but stored in seconds)
        */
        /** Append a binary data element
            @param fieldName name of the field
            @param len length of the binary data in bytes
            @param subtype subtype information for the data. @see enum BinDataType in bsontypes.h.
                   Use BinDataGeneral if you don't care about the type.
            @param data the byte array
        */
        BsonObjectBuilder& appendBinData( const StringData& fieldName, int len, BinDataType type, const void *data ) {
            _b.appendNum( (char) BinData );
            _b.appendStr( fieldName );
            _b.appendNum( len );
            _b.appendNum( (char) type );
            _b.appendBuf( data, len );
            return *this;
        }

        void appendUndefined(const StringData& fieldName) {
            _b.appendNum((char)Undefined);
            _b.appendStr(fieldName);
        }

        /**
        * Append a map of values as a sub-object.
        * Note: the keys of the map should be StringData-compatible (i.e. strings).
        */
        template < class K, class T >
        BsonObjectBuilder& append(const StringData& fieldName, const std::map<K, T >& vals);

        /**
        * destructive
        * The returned BsonObject will free the buffer when it is finished.
        * @return owned BsonObject
        */
        BsonObject obj() {
            return BsonObject(_done());
        }

        /** Fetch the object we have built.
        BsonObjectBuilder still frees the object when the builder goes out of
        scope -- very important to keep in mind.  Use obj() if you
        would like the BsonObject to last longer than the builder.
        */
        BsonObject done() {
            return BsonObject(_done());
        }

        /** Peek at what is in the builder, but leave the builder ready for more appends.
        The returned object is only valid until the next modification or destruction of the builder.
        Intended use case: append a field if not already there.
        */
        BsonObject asTempObj() {
            BsonObject temp(_done());
            _b.setlen(_b.len() - 1); //next append should overwrite the EOO
            _doneCalled = false;
            return temp;
        }

        /** Make it look as if "done" has been called, so that our destructor is a no-op. Do
        *  this if you know that you don't care about the contents of the builder you are
        *  destroying.
        *
        *  Note that it is invalid to call any method other than the destructor after invoking
        *  this method.
        */
        void abandon() {
            _doneCalled = true;
        }

        void appendKeys(const BsonObject& keyPattern, const BsonObject& values);

        static std::string  numStr(int i) {
            if (i >= 0 && i<100 && numStrsReady)
                return numStrs[i];
            StringBuilder o;
            o << i;
            return o.str();
        }

        bool isArray() const {
            return false;
        }

        /** @return true if we are using our own bufbuilder, and not an alternate that was given to us in our constructor */
        bool owned() const { return &_b == &_buf; }

        BsonIterator iterator() const;

        bool hasField(const StringData& name) const;

        int len() const { return _b.len(); }

        BufBuilder& bb() { return _b; }

    private:
        static const std::string numStrs[100]; // cache of 0 to 99 inclusive
        static bool numStrsReady; // for static init safety. see comments in db/jsobj.cpp
    };

    template < class L >
    inline BsonObjectBuilder& _appendIt(BsonObjectBuilder& _this, const StringData& fieldName, const L& vals) {
        BsonObjectBuilder arrBuilder;
        int n = 0;
        for (typename L::const_iterator i = vals.begin(); i != vals.end(); i++)
            arrBuilder.append(BsonObjectBuilder::numStr(n++), *i);
        _this.appendArray(fieldName, arrBuilder.done());
        return _this;
    }

    template < class K, class T >
    inline BsonObjectBuilder& BsonObjectBuilder::append(const StringData& fieldName, const std::map<K, T >& vals) {
        BsonObjectBuilder bob;
        for (typename std::map<K, T>::const_iterator i = vals.begin(); i != vals.end(); ++i){
            bob.append(i->first, i->second);
        }
        append(fieldName, bob.obj());
        return *this;
    }

}
