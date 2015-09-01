# mutt_vc_query

Query vcard file in mutt, using query_command.  Fork of mutt_vc_query, part of
the [Rolo](http://rolo.sourceforge.net/) project, which seems abandoned.

Original read me file provided in README.

## Depends

* [libvc](http://rolo.sourceforge.net/)

## Compilation

    ./configure
    make
    make install

## Usage

    mutt_vc_query -f <path to .vcf> <query>

## Usage in mutt

Add to your muttrc

    set query_command="mutt_vc_query -f <path to .vcf> '%s'"

Then you can press Shift-Q to start a query, or Ctrl-T to complete an email
address. 

## Changes from original

* Added -a option to output all email addresses associated to a contact, instead
  of just the preferred address.
