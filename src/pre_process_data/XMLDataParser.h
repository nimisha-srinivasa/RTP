/** This file is used to generate fragment-based index for multiversion data in XML format.
 *  Author: Nimisha Srinivasa
 **/

#include <libxml++/libxml++.h>
#include "State.h"
#include "Page.h"


class XMLDataParser : public xmlpp::SaxParser
{
public:
  XMLDataParser();
  ~XMLDataParser() override;
  struct stemmer * z;

protected:
  //overrides:
  void on_start_document() override;
  void on_end_document() override;
  void on_start_element(const Glib::ustring& name,
                                const AttributeList& properties) override;
  void on_end_element(const Glib::ustring& name) override;
  void on_characters(const Glib::ustring& characters) override;
  void on_warning(const Glib::ustring& text) override;
  void on_error(const Glib::ustring& text) override;
  void on_fatal_error(const Glib::ustring& text) override;
  
private:
  State curr_state;
  Page* curr_page;
};
 