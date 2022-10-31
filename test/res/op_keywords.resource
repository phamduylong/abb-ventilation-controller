*** Settings ***
Documentation                               Custom keyword to test operation of the page
Library                                     SeleniumLibrary
Resource		                            .${/}auth_keywords.resource

*** Variables ***
${LOG OUT BUTTON ID}                        logout-btn
${SWITCH ID}                                auto-switch
${AUTO PAGE URL}                            http://localhost:3000/auto
${MANUAL PAGE URL}                          http://localhost:3000/manual
${AUTO PAGE TITLE}                          Auto Mode
${MANUAL PAGE TITLE}                        Manual Mode
*** Keywords ***
Login
    Open Browser To Page And Check Title    ${LOGIN URL}    ${LOGIN PAGE TITLE}
    Input Username                          newuser
    Input Account Password                  Ziggs2305
    Submit Credentials
    Auto Mode Page Should Be Open


Logout
    Click Element                           ${LOG OUT BUTTON ID}


Switch To Manual Mode
    Page Should Contain                     ${SWITCH ID}
    Checkbox Should Be Selected             ${SWITCH ID}
    Unselect Checkbox                       ${SWITCH ID}
    Location Should Be                      ${MANUAL PAGE URL}
    Title Should Be                         ${MANUAL PAGE TITLE}


Switch To Auto Mode
    Page Should Contain                     ${SWITCH ID}
    Checkbox Should Not Be Selected         ${SWITCH ID}
    Select Checkbox                         ${SWITCH ID}
    Location Should Be                      ${AUTO PAGE URL}
    Title Should Be                         ${AUTO PAGE TITLE}

    

