#include "crud-wrapper/CrudWrapper.hpp"

#include "gtest/gtest.h"

/// @brief anonymous namespace for needed constants in thus TU
namespace {
/// @brief path to the root of this project
const std::string kprojectRootPath{PROJECT_ROOT_PATH};

} // namespace

/// @brief namespace for arrayAdt_test tests
namespace sql_with_cpp_test::crudWrapper_test {
using namespace ::sql_with_cpp;

TEST(TestingConstruction, ConstructingCrudWrapper) {
  EXPECT_THROW(
      { CrudWrapper{"/non/existing/path"}; },
      std::filesystem::filesystem_error);

  EXPECT_NO_THROW({ CrudWrapper{kprojectRootPath + "/db/album.db"}; });
  EXPECT_NO_THROW({ CrudWrapper{kprojectRootPath + "/db/scratch.db"}; });
  EXPECT_NO_THROW({ CrudWrapper{kprojectRootPath + "/db/world.db"}; });
}

TEST(TestingPeekColumnNames, PeekColumnsNamesOfExistingTablesInAlbumDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/album.db"};
  const auto albumnColumnsNames{db.peekColumnsNames("album")};
  const auto expectedAlbumColumnsNames{
      std::vector<std::string>{"id", "title", "artist", "label", "released"}};
  EXPECT_EQ(albumnColumnsNames, expectedAlbumColumnsNames);

  const auto trackColumnsNames{db.peekColumnsNames("track")};
  const auto expectedTrackColumnsNames{std::vector<std::string>{
      "id", "album_id", "title", "track_number", "duration"}};
  EXPECT_EQ(trackColumnsNames, expectedTrackColumnsNames);
}

TEST(TestingPeekColumnNames,
     PeekColumnsNamesOfExistingTablesInScratchDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/scratch.db"};
  const auto customerColumnsNames{db.peekColumnsNames("customer")};
  const auto expectedCustomerColumnsNames{std::vector<std::string>{
      "id", "name", "address", "city", "state", "zip"}};
  EXPECT_EQ(customerColumnsNames, expectedCustomerColumnsNames);

  const auto domainsColumnsNames{db.peekColumnsNames("domains")};
  const auto expectedDomainsColumnsNames{
      std::vector<std::string>{"id", "domain", "description"}};
  EXPECT_EQ(domainsColumnsNames, expectedDomainsColumnsNames);

  const auto itemColumnsNames{db.peekColumnsNames("item")};
  const auto expectedItemColumnsNames{
      std::vector<std::string>{"id", "name", "description"}};
  EXPECT_EQ(itemColumnsNames, expectedItemColumnsNames);

  const auto saleColumnsNames{db.peekColumnsNames("sale")};
  const auto expectedSaleColumnsNames{std::vector<std::string>{
      "id", "item_id", "customer_id", "date", "quantity", "price"}};
  EXPECT_EQ(saleColumnsNames, expectedSaleColumnsNames);
}

TEST(TestingPeekColumnNames, PeekColumnsNamesOfExistingTablesInWorldDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/world.db"};
  const auto cityColumnsNames{db.peekColumnsNames("City")};
  const auto expectedCityColumnsNames{std::vector<std::string>{
      "ID", "Name", "CountryCode", "District", "Population"}};
  EXPECT_EQ(cityColumnsNames, expectedCityColumnsNames);

  const auto counrtyColumnsNames{db.peekColumnsNames("Country")};
  const auto expectedCounrtyColumnsNames{std::vector<std::string>{
      "Code", "Name", "Continent", "Region", "SurfaceArea", "IndepYear",
      "Population", "LifeExpectancy", "GNP", "GNPOld", "LocalName",
      "GovernmentForm", "HeadOfState", "Capital", "Code2"}};
  EXPECT_EQ(counrtyColumnsNames, expectedCounrtyColumnsNames);

  const auto countryLanguageColumnsNames{
      db.peekColumnsNames("CountryLanguage")};
  const auto expectedCountryLanguageColumnsNames{std::vector<std::string>{
      "CountryCode", "Language", "IsOfficial", "Percentage"}};
  EXPECT_EQ(countryLanguageColumnsNames, expectedCountryLanguageColumnsNames);
}

TEST(TestingPeekColumnNames, PeekColumnsNamesOfNonExistingTables) {
  CrudWrapper const db{kprojectRootPath + "/db/album.db"};
  const auto columnsNames1{db.peekColumnsNames("nonExistingTable1")};
  const auto columnsNames2{db.peekColumnsNames("nonExistingTable2")};
  const auto columnsNames3{db.peekColumnsNames("nonExistingTable3")};

  EXPECT_EQ(columnsNames1, std::vector<std::string>{});
  EXPECT_EQ(columnsNames2, std::vector<std::string>{});
  EXPECT_EQ(columnsNames3, std::vector<std::string>{});
}

TEST(TestingGetRows, GetRowsOfExistingTablesInAlbumDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/album.db"};
  const auto albumRows{db.getRows("album")};
  const auto expectedColumnsNames{
      std::vector<std::string>{"id", "title", "artist", "label", "released"}};

  // check first row which contains columns" names
  EXPECT_EQ(albumRows[0U], expectedColumnsNames);

  // check other rows in the database
  const auto expectedFirstRow{std::vector<std::string>{
      "1", "Two Men with the Blues", "Willie Nelson and Wynton Marsalis",
      "Blue Note", "2008-07-08"}};
  EXPECT_EQ(albumRows[1U], expectedFirstRow);

  const auto expectedSixRow{std::vector<std::string>{
      "17", "Apostrophe", "Frank Zappa", "DiscReet", "1974-04-22"}};
  EXPECT_EQ(albumRows[6U], expectedSixRow);
}

TEST(TestingGetRows, GetRowsOfExistingTablesInScratchDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/scratch.db"};
  const auto albumRows{db.getRows("domains")};
  const auto expectedColumnsNames{
      std::vector<std::string>{"id", "domain", "description"}};

  // check first row which contains columns" names
  EXPECT_EQ(albumRows[0U], expectedColumnsNames);

  // check other rows in the database
  const auto expectedSecondRow{std::vector<std::string>{
      "4", "lynda.com", "Where you go to learn how to do what you do"}};
  EXPECT_EQ(albumRows[2U], expectedSecondRow);

  const auto expectedThirdRow{std::vector<std::string>{
      "5", "google.com", "The place where you go to go to the place"}};
  EXPECT_EQ(albumRows[3U], expectedThirdRow);
}

TEST(TestingGetRows, GetRowsOfExistingTablesInWorldDatabase) {
  CrudWrapper const db{kprojectRootPath + "/db/world.db"};
  const auto albumRows{db.getRows("CountryLanguage")};
  const auto expectedColumnsNames{std::vector<std::string>{
      "CountryCode", "Language", "IsOfficial", "Percentage"}};

  // check first row which contains columns" names
  EXPECT_EQ(albumRows[0U], expectedColumnsNames);

  // check other rows in the database
  const auto expectedTwelfthRow{
      std::vector<std::string>{"ARG", "Spanish", "1", "96.8"}};
  EXPECT_EQ(albumRows[12U], expectedTwelfthRow);

  const auto expectedFourtyForthRow{
      std::vector<std::string>{"EGY", "Arabic", "1", "98.8"}};
  EXPECT_EQ(albumRows[44U], expectedFourtyForthRow);
}

TEST(TestingGetRows, GetRowsOfNonExistingTables) {
  CrudWrapper const db{kprojectRootPath + "/db/scratch.db"};

  const auto rows1{db.getRows("nonExistingTable1")};
  const auto rows2{db.getRows("nonExistingTable2")};
  const auto rows3{db.getRows("nonExistingTable3")};

  EXPECT_EQ(rows1, std::vector<std::vector<std::string>>{{}});
  EXPECT_EQ(rows2, std::vector<std::vector<std::string>>{{}});
  EXPECT_EQ(rows3, std::vector<std::vector<std::string>>{{}});
}

} // namespace sql_with_cpp_test::crudWrapper_test
