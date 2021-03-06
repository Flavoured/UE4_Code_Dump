// Change To The Name you chose when you created the project - eg. #include "ShooterGame.h"
#include "GunCPP.h"
#include "AmmoComponent.h"

// Sets default values for this component's properties
UAmmoComponent::UAmmoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	//bWantsBeginPlay = false;
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts - TODO: Do i need this?
void UAmmoComponent::BeginPlay()
{
	// Check Constructor If Not Working (bWantsBeginPlay)
	Super::BeginPlay();
}

bool UAmmoComponent::Fire()
{
	bool bSuccess = true; // Assume true by default
	if(!IsAmmoDepleted())
	{
		if(IsReloading())
		{
			if (bShootingInteruptsReload)
			{
				CancelReload();
			}
			else
			{
				bSuccess = false;
			}
		}
		else
		{
			TakeAmmo(BulletsPerShot);
			bSuccess = true;
		}
	}
	else
	{
		if(FiringWithNoBulletsStartsReload && CanReload())
		{
			Reload();
		}
		bSuccess = false;
	}
	return bSuccess; // return if shot should be fired or not
}

void UAmmoComponent::Reload()
{
	if(NeedsAndCanReload())
	{
		StartReloading();
	}
}

bool UAmmoComponent::IsReloading()
{
	return (bIsReloading && GetWorld()->GetTimerManager().IsTimerActive(ReloadingHandle));
}

void UAmmoComponent::CancelReload()
{
	if(IsReloading())
	{
		bIsReloading = false;
		GetWorld()->GetTimerManager().ClearTimer(ReloadingHandle);
	}
}

int32 UAmmoComponent::TakeAmmo(int32 Amount)
{
	return SetAmmo(Ammo-Amount);
}

int32 UAmmoComponent::GiveAmmo(int32 Amount)
{
	return SetAmmo(Ammo+Amount);
}

int32 UAmmoComponent::TakeReserveAmmo()
{
	return SetReserveAmmo(ReserveAmmo-1);
}

int32 UAmmoComponent::GiveReserveAmmo(int32 Amount)
{
	return SetReserveAmmo(ReserveAmmo+Amount);
}

int32 UAmmoComponent::SetAmmo(int32 NewAmmo)
{
	Ammo = FMath::Clamp(NewAmmo, 0, MaxAmmo);

	if(Ammo <= 0)
	{
		OnAmmoDepleted.Broadcast();
	}

	return Ammo;
}

int32 UAmmoComponent::SetReserveAmmo(int32 NewReserveAmmo)
{
	ReserveAmmo = FMath::Clamp(NewReserveAmmo, 0, MaxReserveAmmo);

	if(ReserveAmmo <= 0)
	{
		OnReserveDepleted.Broadcast();
	}

	return ReserveAmmo;
}

int32 UAmmoComponent::SetMaxAmmo(int32 NewMaxAmmo)
{
	MaxAmmo = NewMaxAmmo;
	return MaxAmmo;
}

int32 UAmmoComponent::SetMaxReserveAmmo(int32 NewMaxReserveAmmo)
{
	MaxReserveAmmo = NewMaxReserveAmmo;
	return MaxReserveAmmo;
}

float UAmmoComponent::GetReloadTimeLeftPercent()
{
	return GetRemainingReloadTime()/ReloadDuration;
}

bool UAmmoComponent::IsAmmoDepleted()
{
	return Ammo <= 0;
}

bool UAmmoComponent::IsReserveAmmoDepleted()
{
	return ReserveAmmo <= 0;
}

float UAmmoComponent::GetAmmoPercent()
{
	return Ammo/MaxAmmo;
}

float UAmmoComponent::GetRemainingReloadTime()
{
	if(IsReloading() && GetWorld()->GetTimerManager().IsTimerActive(ReloadingHandle))
	{
		return GetWorld()->GetTimerManager().GetTimerRemaining(ReloadingHandle);
	}
	else
	{
		return 0.f;
	}
}

bool UAmmoComponent::NeedsAndCanReload()
{
	return NeedsToReload() && CanReload();
}

FString UAmmoComponent::GetDebugInfo()
{
	return "" + FString::FromInt(Ammo) + " / " + FString::FromInt(ReserveAmmo) + ", Reloading: " + (bIsReloading ? "true" : "false");
}

void UAmmoComponent::ResetAmmo()
{
	SetAmmo(MaxAmmo);
	SetReserveAmmo(MaxReserveAmmo);
}

float UAmmoComponent::SetReloadDuration(float NewReloadDuration)
{
	ReloadDuration = NewReloadDuration;
	return ReloadDuration;
}

bool UAmmoComponent::NeedsToReload()
{
	return Ammo < MaxAmmo;
}

bool UAmmoComponent::CanReload()
{
	return (ReserveAmmo > 0 && !IsReloading());
}

void UAmmoComponent::StartReloading()
{
	if(!IsReloading())
	{
		bIsReloading = true;
		GetWorld()->GetTimerManager().SetTimer(ReloadingHandle, this, &UAmmoComponent::FinishReloading, ReloadDuration);
		OnReloadStart.Broadcast();
	}
}

void UAmmoComponent::FinishReloading()
{
	//if(IsReloading())
	if(bIsReloading)
	{
		if(ReserveAmmo >= (MaxAmmo - Ammo))
		{
			SetReserveAmmo(ReserveAmmo-(MaxAmmo-Ammo));
			SetAmmo(MaxAmmo);
		}
		else
		{
			SetAmmo(ReserveAmmo);
			SetReserveAmmo(0);
		}
		bIsReloading = false;
		OnReloadComplete.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Called without bIsReloading being true"))
	}
}