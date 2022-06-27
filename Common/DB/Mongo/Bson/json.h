/*    Copyright 2014 MongoDB Inc.
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

#include <string>
#include <istream>

namespace Bson {
    class Status;
    class BsonObject;
    class StringData;
    class BsonObjectBuilder;

    /**
     * Create a BsonObject from a JSON <http://www.json.org>,
     * <http://www.ietf.org/rfc/rfc4627.txt> string.  In addition to the JSON
     * extensions extensions described here
     * <http://dochub.mongodb.org/core/mongodbextendedjson>, this function
     * accepts unquoted field names and allows single quotes to optionally be
     * used when specifying field names and string values instead of double
     * quotes.  JSON unicode escape sequences (of the form \uXXXX) are
     * converted to utf8.
     *
     * @throws MsgAssertionException if parsing fails.  The message included with
     * this assertion includes the character offset where parsing failed.
     */
     BsonObject Fromjson(std::istream&, BsonObjectBuilder& builder);

    /** @param len will be size of JSON object in text chars. */
     //BsonObject  Fromjson(const char* str, int* len=NULL);

    /**
     * Parser class.  A BsonObject is constructed incrementally by passing a
     * BsonObjectBuilder to the recursive parsing methods.  The grammar for the
     * element parsed is described before each function.
     */
    class JParse {
        std::istream& _in;
        unsigned long long _offset;

        std::string get(const char *chars_wanted);
    public:
        explicit JParse(std::istream&);

            /*
             * Notation: All-uppercase symbols denote non-terminals; all other
             * symbols are literals.
             */

            /*
             * VALUE :
             *     STRING
             *   | NUMBER
             *   | NUMBERINT
             *   | NUMBERLONG
             *   | OBJECT
             *   | ARRAY
             *
             *   | true
             *   | false
             *   | null
             *   | undefined
             *
             *   | NaN
             *   | Infinity
             *   | -Infinity
             *
             *   | DATE
             *   | TIMESTAMP
             *   | REGEX
             *   | OBJECTID
             *   | DBREF
             *
             *   | new CONSTRUCTOR
             */
        private:
            Bson::Status value(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * OBJECT :
             *     {}
             *   | { MEMBERS }
             *   | SPECIALOBJECT
             *
             * MEMBERS :
             *     PAIR
             *   | PAIR , MEMBERS
             *
             * PAIR :
             *     FIELD : VALUE
             *
             * SPECIALOBJECT :
             *     OIDOBJECT
             *   | BINARYOBJECT
             *   | DATEOBJECT
             *   | TIMESTAMPOBJECT
             *   | REGEXOBJECT
             *   | REFOBJECT
             *   | UNDEFINEDOBJECT
             *   | NUMBERLONGOBJECT
             *
             */
        public:
            Bson::Status object(const StringData& fieldName, BsonObjectBuilder&, bool subObj=true);

        private:
            bool eof() const { return _in.eof(); }
            char getc();

            /* The following functions are called with the '{' and the first
             * field already parsed since they are both implied given the
             * context. */
            /*
             * OIDOBJECT :
             *     { FIELD("$oid") : <24 character hex string> }
             */
            Bson::Status objectIdObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * BINARYOBJECT :
             *     { FIELD("$binary") : <base64 representation of a binary string>,
             *          FIELD("$type") : <hexadecimal representation of a single byte
             *              indicating the data type> }
             */
            Bson::Status binaryObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * DATEOBJECT :
             *     { FIELD("$date") : <64 bit signed integer for milliseconds since epoch> }
             */
            Bson::Status dateObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * TIMESTAMPOBJECT :
             *     { FIELD("$timestamp") : {
             *         FIELD("t") : <32 bit unsigned integer for seconds since epoch>,
             *         FIELD("i") : <32 bit unsigned integer for the increment> } }
             */
            Bson::Status timestampObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             *     NOTE: the rules for the body of the regex are different here,
             *     since it is quoted instead of surrounded by slashes.
             * REGEXOBJECT :
             *     { FIELD("$regex") : <string representing body of regex> }
             *   | { FIELD("$regex") : <string representing body of regex>,
             *          FIELD("$options") : <string representing regex options> }
             */
            Bson::Status regexObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * REFOBJECT :
             *     { FIELD("$ref") : <string representing collection name>,
             *          FIELD("$id") : <24 character hex string> }
             *   | { FIELD("$ref") : STRING , FIELD("$id") : OBJECTID }
             *   | { FIELD("$ref") : STRING , FIELD("$id") : OIDOBJECT }
             */
            Bson::Status dbRefObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * UNDEFINEDOBJECT :
             *     { FIELD("$undefined") : true }
             */
            Bson::Status undefinedObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * NUMBERLONGOBJECT :
             *     { FIELD("$numberLong") : "<number>" }
             */
            Bson::Status numberLongObject(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * ARRAY :
             *     []
             *   | [ ELEMENTS ]
             *
             * ELEMENTS :
             *     VALUE
             *   | VALUE , ELEMENTS
             */
            Bson::Status array(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * NOTE: Currently only Date can be preceded by the "new" keyword
             * CONSTRUCTOR :
             *     DATE
             */
            Bson::Status constructor(const StringData& fieldName, BsonObjectBuilder&);

            /* The following functions only parse the body of the constructor
             * between the parentheses, not including the constructor name */
            /*
             * DATE :
             *     Date( <64 bit signed integer for milliseconds since epoch> )
             */
            Bson::Status date(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * TIMESTAMP :
             *     Timestamp( <32 bit unsigned integer for seconds since epoch>,
             *          <32 bit unsigned integer for the increment> )
             */
            Bson::Status timestamp(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * OBJECTID :
             *     ObjectId( <24 character hex string> )
             */
            Bson::Status objectId(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * NUMBERLONG :
             *     NumberLong( <number> )
             */
            Bson::Status numberLong(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * NUMBERINT :
             *     NumberInt( <number> )
             */
            Bson::Status numberInt(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * DBREF :
             *     Dbref( <namespace string> , <24 character hex string> )
             */
            Bson::Status dbRef(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * REGEX :
             *     / REGEXCHARS / REGEXOPTIONS
             *
             * REGEXCHARS :
             *     REGEXCHAR
             *   | REGEXCHAR REGEXCHARS
             *
             * REGEXCHAR :
             *     any-Unicode-character-except-/-or-\-or-CONTROLCHAR
             *   | \"
             *   | \'
             *   | \\
             *   | \/
             *   | \b
             *   | \f
             *   | \n
             *   | \r
             *   | \t
             *   | \v
             *   | \u HEXDIGIT HEXDIGIT HEXDIGIT HEXDIGIT
             *   | \any-Unicode-character-except-x-or-[0-7]
             *
             * REGEXOPTIONS :
             *     REGEXOPTION
             *   | REGEXOPTION REGEXOPTIONS
             *
             * REGEXOPTION :
             *     g | i | m | s
             */
            Bson::Status regex(const StringData& fieldName, BsonObjectBuilder&);
            Bson::Status regexPat(std::string* result);
            Bson::Status regexOpt(std::string* result);
            Bson::Status regexOptCheck(const StringData& opt);

            /*
             * NUMBER :
             *
             * NOTE: Number parsing is based on standard library functions, not
             * necessarily on the JSON numeric grammar.
             *
             * Number as value - strtoll and strtod
             * Date - strtoll
             * Timestamp - strtoul for both timestamp and increment and '-'
             * before a number explicity disallowed
             */
            Bson::Status number(const StringData& fieldName, BsonObjectBuilder&);

            /*
             * FIELD :
             *     STRING
             *   | [a-zA-Z$_] FIELDCHARS
             *
             * FIELDCHARS :
             *     [a-zA-Z0-9$_]
             *   | [a-zA-Z0-9$_] FIELDCHARS
             */
            Bson::Status field(std::string* result);

            /*
             * STRING :
             *     " "
             *   | ' '
             *   | " CHARS "
             *   | ' CHARS '
             */
            Bson::Status quotedString(std::string* result);

            /*
             * CHARS :
             *     CHAR
             *   | CHAR CHARS
             *
             * Note: " or ' may be allowed depending on whether the string is
             * double or single quoted
             *
             * CHAR :
             *     any-Unicode-character-except-"-or-'-or-\-or-CONTROLCHAR
             *   | \"
             *   | \'
             *   | \\
             *   | \/
             *   | \b
             *   | \f
             *   | \n
             *   | \r
             *   | \t
             *   | \v
             *   | \u HEXDIGIT HEXDIGIT HEXDIGIT HEXDIGIT
             *   | \any-Unicode-character-except-x-or-[0-9]
             *
             * HEXDIGIT : [0..9a..fA..F]
             *
             * per http://www.ietf.org/rfc/rfc4627.txt, control characters are
             * (U+0000 through U+001F).  U+007F is not mentioned as a control
             * character.
             * CONTROLCHAR : [0x00..0x1F]
             *
             * If there is not an error, result will contain a null terminated
             * string, but there is no guarantee that it will not contain other
             * null characters.
             */
            Bson::Status chars(std::string* result, const char* terminalSet, const char* allowedSet=NULL);

            /**
             * Converts the two byte Unicode code point to its UTF8 character
             * encoding representation.  This function returns a string because
             * UTF8 encodings for code points from 0x0000 to 0xFFFF can range
             * from one to three characters.
             */
            std::string encodeUTF8(unsigned char first, unsigned char second) const;

            /**
             * @return true if the given token matches the next non whitespace
             * sequence in our buffer, and false if the token doesn't match or
             * we reach the end of our buffer.  Do not update the pointer to our
             * buffer (same as calling readTokenImpl with advance=false).
             */
            inline bool peekToken(const char* token);

            char peek();

            /**
             * @return true if the given token matches the next non whitespace
             * sequence in our buffer, and false if the token doesn't match or
             * we reach the end of our buffer.  Updates the pointer to our
             * buffer (same as calling readTokenImpl with advance=true).
             */
            inline bool readToken(const char* token);

            /**
             * @return true if the given token matches the next non whitespace
             * sequence in our buffer, and false if the token doesn't match or
             * we reach the end of our buffer.  Do not update the pointer to our
             * buffer if advance is false.
             */
            bool readTokenImpl(const char* token);

            /**
             * @return true if the next field in our stream matches field.
             * Handles single quoted, double quoted, and unquoted field names
             */
            bool readField(const StringData& field);

            /**
             * @return true if matchChar is in matchSet
             * @return true if matchSet is NULL and false if it is an empty string
             */
            bool match(char matchChar, const char* matchSet) const;

            /**
             * @return true if every character in the string is a hex digit
             */
            bool isHexString(const StringData&) const;

            /**
             * @return FailedToParse status with the given message and some
             * additional context information
             */
            Bson::Status parseError(const StringData& msg);
        public:
            inline long long offset() { return _offset; }

        private:
            Status err();
            /*
             * _buf - start of our input buffer
             * _input - cursor we advance in our input buffer
             * _input_end - sentinel for the end of our input buffer
             *
             * _buf is the null terminated buffer containing the JSON string we
             * are parsing.  _input_end points to the null byte at the end of
             * the buffer.  strtoll, strtol, and strtod will access the null
             * byte at the end of the buffer because they are assuming a c-style
             * string.
             */
            //const char* const _buf;
            //const char* _input;
            //const char* const _input_end;
    };

} // namespace mongo
