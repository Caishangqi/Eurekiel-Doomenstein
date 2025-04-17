#pragma once
#include "Game/Framework/Widget.hpp"

class WidgetLobby : public Widget
{
public:
    WidgetLobby();
    ~WidgetLobby() override;

public:
    void Draw() const override;
    void Update() override;

    /// Setter

protected:
    void UpdateKeyInput();
    void RemoveAllLocalPlayerControllers();
    void HandleLocalPlayerViewportData();
    void HandleGameStartProcess();

private:
    std::vector<std::string> GetPlayerActionNamesByDeviceType(DeviceType deviceType) const;
};
