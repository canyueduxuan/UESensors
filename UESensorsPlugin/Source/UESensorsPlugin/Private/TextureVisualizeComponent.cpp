// Fill out your copyright notice in the Description page of Project Settings.


#include "TextureVisualizeComponent.h"

// Sets default values for this component's properties
UTextureVisualizeComponent::UTextureVisualizeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTextureVisualizeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UTextureVisualizeComponent::AttachImage);
}


// Called every frame
void UTextureVisualizeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTextureVisualizeComponent::AttachImage()
{
	TArray<UUserWidget*> Widgets;

	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        GetWorld(),
        Widgets,
        UUserWidget::StaticClass(),
        false      // 只获取Viewport中的
    );

    for (UUserWidget* Widget : Widgets)
    {
        if (!Widget) continue;

		UImage* Img = Cast<UImage>(Widget->GetWidgetFromName(ImageName));
        if (Img)
        {
			AActor* Owner = GetOwner();
            if (Owner && Owner->GetClass()->ImplementsInterface(UTextureVisualizeInterface::StaticClass()))
			{
				UMaterialInterface* Material = ITextureVisualizeInterface::Execute_GetCameraMaterial(Owner);

				if (Material)
				{
					Img->SetBrushFromMaterial(Material);
				}
			}
        }
    }
}
