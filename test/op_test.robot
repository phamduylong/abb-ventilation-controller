*** Settings ***
Documentation                               Testing authenticate functions
Library                                     SeleniumLibrary
Library                                     FakerLibrary  locale=fi_FI
Resource		                            .${/}res${/}op_keywords.resource
Test Setup                                  Open Browser To Page And Check Title    ${SIGNUP URL}   ${SIGNUP PAGE TITLE}
Test Teardown                               Close Browser

*** Variables ***
${RAND USERNAME}
${RAND PASSWORD}

*** Test Cases ***

Change Mode In Auto Mode Page
    Login
    Switch To Manual Mode
    Logout

