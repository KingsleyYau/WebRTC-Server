#include "json/writer.h"
#include <utility>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace Json {
/**
 * Modify by Max 2019/10/31
 * For output UNICODE text string, but not original char *, just output like \ud83d\ude03, but not output EMOJI or Chinese
 */
static bool containsMultiByte( const char* str ) {
	while ( *str ) {
		if ( ( *(str++) ) & 0x80 ) {
			return true;
		}
	}
	return false;
}

/**
 * Modify by Max 2019/10/31
 * For output UNICODE text string, but not original char *, just output like \ud83d\ude03, but not output EMOJI or Chinese
 */
static int UTF8TocodePoint(const char *c, unsigned int *result) {
	int count = 0;

	if (((*c) & 0x80) == 0) {
		*result = static_cast<unsigned int>(*c);
		count = 1;
	} else if (((*c) & 0xe0) == 0xc0) {
		*result = static_cast<unsigned int>((((*c) & 0x1f) << 6) | ((*(c + 1)) & 0x3f));
		count = 2;
	} else if (((*c) & 0xf0) == 0xe0) {
		*result = static_cast<unsigned int>((((*c) & 0xf) << 12) | (((*(c + 1)) & 0x3f) << 6) | (((*(c + 2)) & 0x3f)));
		count = 3;
	} else if (((*c) & 0xf8) == 0xf0) {
		*result = static_cast<unsigned int>((((*c) & 0x7) << 18) | (((*(c + 1)) & 0x3f) << 12) | (((*(c + 2)) & 0x3f) << 6) | (((*(c + 3)) & 0x3f)));
		count = 4;
	} else if (((*c) & 0xfc) == 0xf8) {
		*result = static_cast<unsigned int>((((*c) & 0x3) << 24) | (((*(c + 1)) & 0x3f) << 18) | (((*(c + 2)) & 0x3f) << 12) | (((*(c + 3)) & 0x3f) << 6) | (((*(c + 4)) & 0x3f)));
		count = 5;
	} else if (((*c) & 0xfe) == 0xfc) {
		*result = static_cast<unsigned int>((((*c) & 0x1) << 30) | (((*(c + 1)) & 0x3f) << 24) | (((*(c + 2)) & 0x3f) << 18) | (((*(c + 3)) & 0x3f) << 12) | (((*(c + 4)) & 0x3f) << 6) | (((*(c + 5)) & 0x3f)));
		count = 6;
	}

	return count;
}

static bool isControlCharacter(char ch)
{
   return ch > 0 && ch <= 0x1F;
}

static bool containsControlCharacter( const char* str )
{
   while ( *str ) 
   {
      if ( isControlCharacter( *(str++) ) )
         return true;
   }
   return false;
}
static void uintToString( unsigned int value, 
                          char *&current )
{
   *--current = 0;
   do
   {
      *--current = (value % 10) + '0';
      value /= 10;
   }
   while ( value != 0 );
}

std::string valueToString( Int value )
{
   char buffer[32];
   char *current = buffer + sizeof(buffer);
   bool isNegative = value < 0;
   if ( isNegative )
      value = -value;
   uintToString( UInt(value), current );
   if ( isNegative )
      *--current = '-';
   assert( current >= buffer );
   return current;
}


std::string valueToString( UInt value )
{
   char buffer[32];
   char *current = buffer + sizeof(buffer);
   uintToString( value, current );
   assert( current >= buffer );
   return current;
}

std::string valueToString( double value )
{
   char buffer[32];
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__) // Use secure version with visual studio 2005 to avoid warning. 
   sprintf_s(buffer, sizeof(buffer), "%#.16g", value); 
#else	
   sprintf(buffer, "%#.16g", value); 
#endif
   char* ch = buffer + strlen(buffer) - 1;
   if (*ch != '0') return buffer; // nothing to truncate, so save time
   while(ch > buffer && *ch == '0'){
     --ch;
   }
   char* last_nonzero = ch;
   while(ch >= buffer){
     switch(*ch){
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9':
       --ch;
       continue;
     case '.':
       // Truncate zeroes to save bytes in output, but keep one.
       *(last_nonzero+2) = '\0';
       return buffer;
     default:
       return buffer;
     }
   }
   return buffer;
}


std::string valueToString( bool value )
{
   return value ? "true" : "false";
}

std::string valueToQuotedString( const char *value, bool encodeToUnicodeString )
{
	/**
	 * Modify by Max 2019/10/31
	 * For output UNICODE text string, but not original char *, just output like \ud83d\ude03, but not output EMOJI or Chinese
	 */
	if ( !encodeToUnicodeString ) {
		// Not sure how to handle unicode...
		if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter( value ))
			return std::string("\"") + value + "\"";
	} else {
	   if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter( value ) && !containsMultiByte( value ))
		  return std::string("\"") + value + "\"";
	}

   // We have to walk value and escape any special characters.
   // Appending to std::string is not efficient, but this should be rare.
   // (Note: forward slashes are *not* rare, but I am not escaping them.)
   unsigned maxsize = strlen(value)*2 + 3; // allescaped+quotes+NULL
   std::string result;
   result.reserve(maxsize); // to avoid lots of mallocs
   result += "\"";
   for (const char* c=value; *c != 0; ++c)
   {
      switch(*c)
      {
         case '\"':
            result += "\\\"";
            break;
         case '\\':
            result += "\\\\";
            break;
         case '\b':
            result += "\\b";
            break;
         case '\f':
            result += "\\f";
            break;
         case '\n':
            result += "\\n";
            break;
         case '\r':
            result += "\\r";
            break;
         case '\t':
            result += "\\t";
            break;
         //case '/':
            // Even though \/ is considered a legal escape in JSON, a bare
            // slash is also legal, so I see no reason to escape it.
            // (I hope I am not misunderstanding something.
            // blep notes: actually escaping \/ may be useful in javascript to avoid </ 
            // sequence.
            // Should add a flag to allow this compatibility mode and prevent this 
            // sequence from occurring.
         default:
            if ( isControlCharacter( *c ) )
            {
               std::ostringstream oss;
               oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(*c);
               result += oss.str();
            }
            else
            {
            	/**
            	 * Modify by Max 2019/10/31
            	 * For output UNICODE text string, but not original char *, just output like \ud83d\ude03, but not output EMOJI or Chinese
            	 */
            	if ( !encodeToUnicodeString ) {
            		result += *c;
            	} else {
            		if ((*c) & 0x80) {
                    	unsigned int num = 0;
                    	c += UTF8TocodePoint(c, &num) - 1;
//                    	printf("# c: 0x%02x, num: 0x%x \n", (unsigned char)*c, num);
                    	std::ostringstream oss;
                    	if ( num >= 0x1F300 && num <= 0x1F3FF ) {
                    		oss << "\\uD83C";
                    		int tmp = num - 0x1F300 + 0xDF00;
                    		oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(tmp);
                    	} else if ( num >= 0x1F400 && num <= 0x1F7FF ) {
                    		oss << "\\uD83D";
                    		int tmp = num - 0x1F400 + 0xDC00;
                    		oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(tmp);
                    	} else {
                        	oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(num);
                    	}
                    	result += oss.str();
//                    	printf("# oss: %s \n", oss.str().c_str());
            		} else {
            			result += *c;
            		}
            	}
            }
//            else {
//            	result += *c;
//            }
            break;
      }
   }
   result += "\"";
   return result;
}

// Class Writer
// //////////////////////////////////////////////////////////////////
Writer::~Writer()
{
}


// Class FastWriter
// //////////////////////////////////////////////////////////////////

FastWriter::FastWriter( bool encodeToUnicodeString )
   : yamlCompatiblityEnabled_( false )
{
	encodeToUnicodeString_ = encodeToUnicodeString;
}


void 
FastWriter::enableYAMLCompatibility()
{
   yamlCompatiblityEnabled_ = true;
}


std::string 
FastWriter::write( const Value &root )
{
   document_ = "";
   writeValue( root );
   document_ += "\n";
   return document_;
}


void 
FastWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      document_ += "null";
      break;
   case intValue:
      document_ += valueToString( value.asInt() );
      break;
   case uintValue:
      document_ += valueToString( value.asUInt() );
      break;
   case realValue:
      document_ += valueToString( value.asDouble() );
      break;
   case stringValue:
      document_ += valueToQuotedString( value.asCString(), encodeToUnicodeString_ );
      break;
   case booleanValue:
      document_ += valueToString( value.asBool() );
      break;
   case arrayValue:
      {
         document_ += "[";
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ",";
            writeValue( value[index] );
         }
         document_ += "]";
      }
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         document_ += "{";
         for ( Value::Members::iterator it = members.begin(); 
               it != members.end(); 
               ++it )
         {
            const std::string &name = *it;
            if ( it != members.begin() )
               document_ += ",";
            document_ += valueToQuotedString( name.c_str() );
            document_ += yamlCompatiblityEnabled_ ? ": " 
                                                  : ":";
            writeValue( value[name] );
         }
         document_ += "}";
      }
      break;
   }
}


// Class StyledWriter
// //////////////////////////////////////////////////////////////////

StyledWriter::StyledWriter()
   : rightMargin_( 74 )
   , indentSize_( 3 )
{
}


std::string 
StyledWriter::write( const Value &root )
{
   document_ = "";
   addChildValues_ = false;
   indentString_ = "";
   writeCommentBeforeValue( root );
   writeValue( root );
   writeCommentAfterValueOnSameLine( root );
   document_ += "\n";
   return document_;
}


void 
StyledWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( "null" );
      break;
   case intValue:
      pushValue( valueToString( value.asInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         if ( members.empty() )
            pushValue( "{}" );
         else
         {
            writeWithIndent( "{" );
            indent();
            Value::Members::iterator it = members.begin();
            while ( true )
            {
               const std::string &name = *it;
               const Value &childValue = value[name];
               writeCommentBeforeValue( childValue );
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               document_ += " : ";
               writeValue( childValue );
               if ( ++it == members.end() )
               {
                  writeCommentAfterValueOnSameLine( childValue );
                  break;
               }
               document_ += ",";
               writeCommentAfterValueOnSameLine( childValue );
            }
            unindent();
            writeWithIndent( "}" );
         }
      }
      break;
   }
}


void 
StyledWriter::writeArrayValue( const Value &value )
{
   unsigned size = value.size();
   if ( size == 0 )
      pushValue( "[]" );
   else
   {
      bool isArrayMultiLine = isMultineArray( value );
      if ( isArrayMultiLine )
      {
         writeWithIndent( "[" );
         indent();
         bool hasChildValue = !childValues_.empty();
         unsigned index =0;
         while ( true )
         {
            const Value &childValue = value[index];
            writeCommentBeforeValue( childValue );
            if ( hasChildValue )
               writeWithIndent( childValues_[index] );
            else
            {
               writeIndent();
               writeValue( childValue );
            }
            if ( ++index == size )
            {
               writeCommentAfterValueOnSameLine( childValue );
               break;
            }
            document_ += ",";
            writeCommentAfterValueOnSameLine( childValue );
         }
         unindent();
         writeWithIndent( "]" );
      }
      else // output on a single line
      {
         assert( childValues_.size() == size );
         document_ += "[ ";
         for ( unsigned index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ += ", ";
            document_ += childValues_[index];
         }
         document_ += " ]";
      }
   }
}


bool 
StyledWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();
   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }
   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'
      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
         isMultiLine = isMultiLine  &&  hasCommentForValue( value[index] );
      }
      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}


void 
StyledWriter::pushValue( const std::string &value )
{
   if ( addChildValues_ )
      childValues_.push_back( value );
   else
      document_ += value;
}


void 
StyledWriter::writeIndent()
{
   if ( !document_.empty() )
   {
      char last = document_[document_.length()-1];
      if ( last == ' ' )     // already indented
         return;
      if ( last != '\n' )    // Comments may add new-line
         document_ += '\n';
   }
   document_ += indentString_;
}


void 
StyledWriter::writeWithIndent( const std::string &value )
{
   writeIndent();
   document_ += value;
}


void 
StyledWriter::indent()
{
   indentString_ += std::string( indentSize_, ' ' );
}


void 
StyledWriter::unindent()
{
   assert( int(indentString_.size()) >= indentSize_ );
   indentString_.resize( indentString_.size() - indentSize_ );
}


void 
StyledWriter::writeCommentBeforeValue( const Value &root )
{
   if ( !root.hasComment( commentBefore ) )
      return;
   document_ += normalizeEOL( root.getComment( commentBefore ) );
   document_ += "\n";
}


void 
StyledWriter::writeCommentAfterValueOnSameLine( const Value &root )
{
   if ( root.hasComment( commentAfterOnSameLine ) )
      document_ += " " + normalizeEOL( root.getComment( commentAfterOnSameLine ) );

   if ( root.hasComment( commentAfter ) )
   {
      document_ += "\n";
      document_ += normalizeEOL( root.getComment( commentAfter ) );
      document_ += "\n";
   }
}


bool 
StyledWriter::hasCommentForValue( const Value &value )
{
   return value.hasComment( commentBefore )
          ||  value.hasComment( commentAfterOnSameLine )
          ||  value.hasComment( commentAfter );
}


std::string 
StyledWriter::normalizeEOL( const std::string &text )
{
   std::string normalized;
   normalized.reserve( text.length() );
   const char *begin = text.c_str();
   const char *end = begin + text.length();
   const char *current = begin;
   while ( current != end )
   {
      char c = *current++;
      if ( c == '\r' ) // mac or dos EOL
      {
         if ( *current == '\n' ) // convert dos EOL
            ++current;
         normalized += '\n';
      }
      else // handle unix EOL & other char
         normalized += c;
   }
   return normalized;
}


// Class StyledStreamWriter
// //////////////////////////////////////////////////////////////////

StyledStreamWriter::StyledStreamWriter( std::string indentation )
   : document_(NULL)
   , rightMargin_( 74 )
   , indentation_( indentation )
{
}


void
StyledStreamWriter::write( std::ostream &out, const Value &root )
{
   document_ = &out;
   addChildValues_ = false;
   indentString_ = "";
   writeCommentBeforeValue( root );
   writeValue( root );
   writeCommentAfterValueOnSameLine( root );
   *document_ << "\n";
   document_ = NULL; // Forget the stream, for safety.
}


void 
StyledStreamWriter::writeValue( const Value &value )
{
   switch ( value.type() )
   {
   case nullValue:
      pushValue( "null" );
      break;
   case intValue:
      pushValue( valueToString( value.asInt() ) );
      break;
   case uintValue:
      pushValue( valueToString( value.asUInt() ) );
      break;
   case realValue:
      pushValue( valueToString( value.asDouble() ) );
      break;
   case stringValue:
      pushValue( valueToQuotedString( value.asCString() ) );
      break;
   case booleanValue:
      pushValue( valueToString( value.asBool() ) );
      break;
   case arrayValue:
      writeArrayValue( value);
      break;
   case objectValue:
      {
         Value::Members members( value.getMemberNames() );
         if ( members.empty() )
            pushValue( "{}" );
         else
         {
            writeWithIndent( "{" );
            indent();
            Value::Members::iterator it = members.begin();
            while ( true )
            {
               const std::string &name = *it;
               const Value &childValue = value[name];
               writeCommentBeforeValue( childValue );
               writeWithIndent( valueToQuotedString( name.c_str() ) );
               *document_ << " : ";
               writeValue( childValue );
               if ( ++it == members.end() )
               {
                  writeCommentAfterValueOnSameLine( childValue );
                  break;
               }
               *document_ << ",";
               writeCommentAfterValueOnSameLine( childValue );
            }
            unindent();
            writeWithIndent( "}" );
         }
      }
      break;
   }
}


void 
StyledStreamWriter::writeArrayValue( const Value &value )
{
   unsigned size = value.size();
   if ( size == 0 )
      pushValue( "[]" );
   else
   {
      bool isArrayMultiLine = isMultineArray( value );
      if ( isArrayMultiLine )
      {
         writeWithIndent( "[" );
         indent();
         bool hasChildValue = !childValues_.empty();
         unsigned index =0;
         while ( true )
         {
            const Value &childValue = value[index];
            writeCommentBeforeValue( childValue );
            if ( hasChildValue )
               writeWithIndent( childValues_[index] );
            else
            {
	       writeIndent();
               writeValue( childValue );
            }
            if ( ++index == size )
            {
               writeCommentAfterValueOnSameLine( childValue );
               break;
            }
            *document_ << ",";
            writeCommentAfterValueOnSameLine( childValue );
         }
         unindent();
         writeWithIndent( "]" );
      }
      else // output on a single line
      {
         assert( childValues_.size() == size );
         *document_ << "[ ";
         for ( unsigned index =0; index < size; ++index )
         {
            if ( index > 0 )
               *document_ << ", ";
            *document_ << childValues_[index];
         }
         *document_ << " ]";
      }
   }
}


bool 
StyledStreamWriter::isMultineArray( const Value &value )
{
   int size = value.size();
   bool isMultiLine = size*3 >= rightMargin_ ;
   childValues_.clear();
   for ( int index =0; index < size  &&  !isMultiLine; ++index )
   {
      const Value &childValue = value[index];
      isMultiLine = isMultiLine  ||
                     ( (childValue.isArray()  ||  childValue.isObject())  &&  
                        childValue.size() > 0 );
   }
   if ( !isMultiLine ) // check if line length > max line length
   {
      childValues_.reserve( size );
      addChildValues_ = true;
      int lineLength = 4 + (size-1)*2; // '[ ' + ', '*n + ' ]'
      for ( int index =0; index < size  &&  !isMultiLine; ++index )
      {
         writeValue( value[index] );
         lineLength += int( childValues_[index].length() );
         isMultiLine = isMultiLine  &&  hasCommentForValue( value[index] );
      }
      addChildValues_ = false;
      isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
   }
   return isMultiLine;
}


void 
StyledStreamWriter::pushValue( const std::string &value )
{
   if ( addChildValues_ )
      childValues_.push_back( value );
   else
      *document_ << value;
}


void 
StyledStreamWriter::writeIndent()
{
  /*
    Some comments in this method would have been nice. ;-)

   if ( !document_.empty() )
   {
      char last = document_[document_.length()-1];
      if ( last == ' ' )     // already indented
         return;
      if ( last != '\n' )    // Comments may add new-line
         *document_ << '\n';
   }
  */
   *document_ << '\n' << indentString_;
}


void 
StyledStreamWriter::writeWithIndent( const std::string &value )
{
   writeIndent();
   *document_ << value;
}


void 
StyledStreamWriter::indent()
{
   indentString_ += indentation_;
}


void 
StyledStreamWriter::unindent()
{
   assert( indentString_.size() >= indentation_.size() );
   indentString_.resize( indentString_.size() - indentation_.size() );
}


void 
StyledStreamWriter::writeCommentBeforeValue( const Value &root )
{
   if ( !root.hasComment( commentBefore ) )
      return;
   *document_ << normalizeEOL( root.getComment( commentBefore ) );
   *document_ << "\n";
}


void 
StyledStreamWriter::writeCommentAfterValueOnSameLine( const Value &root )
{
   if ( root.hasComment( commentAfterOnSameLine ) )
      *document_ << " " + normalizeEOL( root.getComment( commentAfterOnSameLine ) );

   if ( root.hasComment( commentAfter ) )
   {
      *document_ << "\n";
      *document_ << normalizeEOL( root.getComment( commentAfter ) );
      *document_ << "\n";
   }
}


bool 
StyledStreamWriter::hasCommentForValue( const Value &value )
{
   return value.hasComment( commentBefore )
          ||  value.hasComment( commentAfterOnSameLine )
          ||  value.hasComment( commentAfter );
}


std::string 
StyledStreamWriter::normalizeEOL( const std::string &text )
{
   std::string normalized;
   normalized.reserve( text.length() );
   const char *begin = text.c_str();
   const char *end = begin + text.length();
   const char *current = begin;
   while ( current != end )
   {
      char c = *current++;
      if ( c == '\r' ) // mac or dos EOL
      {
         if ( *current == '\n' ) // convert dos EOL
            ++current;
         normalized += '\n';
      }
      else // handle unix EOL & other char
         normalized += c;
   }
   return normalized;
}


std::ostream& operator<<( std::ostream &sout, const Value &root )
{
   Json::StyledStreamWriter writer;
   writer.write(sout, root);
   return sout;
}


} // namespace Json
