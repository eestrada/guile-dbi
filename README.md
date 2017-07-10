Guile DBI
=========

guile-dbi provides a simple, generic, easy-to-use scheme/guile
interface to SQL databases, such as Postgres, MySQL or SQLite.

The system is 'generic' in the sense that the same programming interface
(front end) can be used with different databases. The DBI (database
independent) part provides the scheme interfaces. The DBD (database
dependent) plugins connect to an actual SQL server. Currently, there are
DBD back-ends for Postgres, MySQL and SQLite3. Creating additional DBD
back-ends requires a small amount of C coding, but is a straightforward
task.

Guile-dbi is simple - which is both a blessing and a curse. For the most
part, all that it does is to accept guile strings encoding SQL
statements, forward these to the database, and return rows as scheme
association lists. A minimal amount of translation is performed - for
example, SQL floating point columns are converted to scheme floating
point numbers - but there is nothing fancier than this - there is no
special treatment for dates, currencies, etc. At this time, there is no
support for prepared statements. Capable programmers are invited to add
support for this and other missing features.

Documentation
-------------
The guile-dbi user-manual and reference is
[here](http://htmlpreview.github.com/?https://github.com/opencog/guile-dbi/blob/master/website/guile-dbi.html).

A copy of the old, defunct website is
[here](http://htmlpreview.github.com/?https://github.com/opencog/guile-dbi/blob/master/website/index.html).

Mailing List
------------
All discussion of guile-dbi should be directed to: guile-user@gnu.org.

License
-------
Guile-dbi is distributed under the Gnu GPLv2 license. Code and website
were developed by Maurizio Boriani (2005-2006) and are currently
maintained by Linas Vepstas (2008-2010).
