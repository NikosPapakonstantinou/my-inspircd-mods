/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *       Copyright (C) Nikos `UrL` Papakonstantinou <url.euro@gmail.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.      If not, see <http://www.gnu.org/licenses/>.
 */
 
 #include "inspircd.h"
 #include "xline.h"
/* $ModConfig: <ctcpxline pattern="mirc*" action="gline" duration="84600" reason="You version is blacklisted on this server."> */
/* $ModDesc: Bans specific ctcp replies. */
/* $ModAuthor: Nikos `UrL` Papakonstantinou */
/* $ModAuthorMail: url@mirc.com.gr */
/* $ModDepends: core 2.0-2.1 */
#define DO_KILL     0
#define DO_ZLINE    1
#define DO_GLINE    2
class ModuleCtcpXline : public Module
{
  std::string cxPattern;
  // std::string cxAction;
  std::string cxReason;
  unsigned int cxAction;
  unsigned int cxDuration;
  unsigned int duration;

 public:
        void init()
        {
                Implementation eventlist[] = { I_OnUserRegister, I_OnPreCommand, I_OnRehash };
                ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist)/sizeof(Implementation));
                OnRehash(NULL);
        }
        Version GetVersion()
        {
                return Version("Bans a specific ctcp version.");
        }
        void OnRehash(User* user)
        {
                std::string reader;

                reader = ServerInstance->Config->ConfValue("ctcpxline")->getString("pattern");
                if (!reader.empty())
                {
                  this->cxPattern = reader;
                }
                else
                {
                        ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid pattern value in config: %s", reader.c_str());
                        ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid pattern value in config: %s", reader.c_str());
                }

                reader = ServerInstance->Config->ConfValue("ctcpxline")->getString("action");
                if (reader == "GLINE")
                {
                        this->cxAction = DO_GLINE;
                }
                else if (reader == "ZLINE")
                {
                        this->cxAction = DO_ZLINE;
                }
                else if (reader == "KILL")
                {
                        this->cxAction = DO_KILL;
                }
                else
                {
                        ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid action value in config: %s", reader.c_str());
                        ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid action value in config: %s", reader.c_str());
                }
                reader = ServerInstance->Config->ConfValue("ctcpxline")->getString("duration");
                if (!reader.empty())
                {
                        this->cxDuration = ServerInstance->Duration(reader.c_str());
                        if ((int)this->duration <= 0)
                        {
                                ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
                                ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
                        }
                }
                else
                {
                        ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
                        ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid duration value in config: %s", reader.c_str());
                }

                reader = ServerInstance->Config->ConfValue("ctcpxline")->getString("reason");
                if (!reader.empty())
                {
                        this->cxReason = reader;
                }
                else
                {
                        ServerInstance->SNO->WriteGlobalSno('a', "m_ctcpxline: Invalid reason value in config: %s", reader.c_str());
                        ServerInstance->Logs->Log("CONFIG",DEFAULT, "m_ctcpxline: Invalid reason value in config: %s", reader.c_str());
                }
        }

        ModResult OnUserRegister(LocalUser* user)
        {
                std::string ctcpmethod = "VERSION";
                user->WriteServ("PRIVMSG %s :\001%s\001", user->nick.c_str(), ctcpmethod.c_str());
                return MOD_RES_PASSTHRU;
        }


        ModResult OnPreCommand(std::string &command, std::vector<std::string> &parameters, LocalUser* user, bool validated, const std::string &original_line)
        {
                // TODO: configtaglist to handle more patterns 
                // ConfigTagList cxconftag = ServerInstance->Config->ConfTags("ctcpxline");
                // for (ConfigIter i = cxconftag.first; i != cxconftag.second; ++i)
                if (command == "NOTICE" && !validated && parameters.size() > 1)
                {
                        if (InspIRCd::Match(parameters[1], cxPattern, ascii_case_insensitive_map))
                        {
                                switch (this->cxAction)
                                {
                                        case DO_KILL:
                                        {
                                                ServerInstance->Users->QuitUser(user, this->cxReason);
                                                break;
                                        }
                                        case DO_ZLINE:
                                        {
                                                ZLine* cxzline = new ZLine(ServerInstance->Time(), this->cxDuration, ServerInstance->Config->ServerName, this->cxReason.c_str(), user->GetIPString());
                                                if (ServerInstance->XLines->AddLine(cxzline,user))
                                                        ServerInstance->XLines->ApplyLines();
                                                else
                                                        delete cxzline;
                                                break;
                                        }
                                        case DO_GLINE:
                                        {
                                                GLine* cxgline = new GLine(ServerInstance->Time(), this->cxDuration, ServerInstance->Config->ServerName, this->cxReason.c_str(), "*", user->GetIPString());
                                                if (ServerInstance->XLines->AddLine(cxgline,user))
                                                        ServerInstance->XLines->ApplyLines();
                                                else
                                                        delete cxgline;
                                                break;
                                        }
                                }
                        }
                        else
                        {
                                return MOD_RES_PASSTHRU;
                        }
                }
        return MOD_RES_PASSTHRU;
        }

};
MODULE_INIT(ModuleCtcpXline)
