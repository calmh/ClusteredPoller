#include "Failure.h"

#include <stdio.h>
#include <string.h>

Failure::Failure (const SimpleString&	theTestName,
				  const SimpleString&	theFileName,
		          long	 				theLineNumber,
		          const SimpleString&	theCondition)
: message (theCondition),
  testName (theTestName),
  fileName (theFileName),
  lineNumber (theLineNumber)
{
}


Failure::Failure (const SimpleString&	theTestName,
			 	  const SimpleString&	theFileName,
				  long					theLineNumber,
				  const SimpleString&	expected,
				  const SimpleString&	actual)
: testName (theTestName),
  fileName (theFileName),
  lineNumber (theLineNumber)
{
	char *part1 = (char*)"expected ";
	char *part3 = (char*)" but was: ";

	char *stage = new char [strlen (part1)
					+ expected.size ()
					+ strlen (part3)
					+ actual.size ()
					+ 1];

	sprintf(stage, "%s%s%s%s",
		part1,
		expected.asCharString(),
		part3,
		actual.asCharString());

	message = SimpleString(stage);

	delete stage;
}


